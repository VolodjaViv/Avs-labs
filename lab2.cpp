

#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <chrono>

using namespace std;

//----------------task_1------------------------------------------------------------------------------------------------

// базовый класс для счетчиков
class Counter {
public:
    Counter(const int &dataSize) {
        _size = dataSize;
        _id = 0;
    }

    bool canContinue() {
        return _id < _size;
    }

    void reset() {
        _id = 0;
    }

    virtual int next() = 0; // абстрактный метод для следующего необработанного индекса
protected:
    int _id;
    int _size;
};

//------------

class Atomic : public Counter {
public:
    Atomic(const int &dataSize) : Counter(dataSize) {
        _busy = false;
    }

    int next() override{
        int id;
        while (true) {
            bool expected = false;
            if (!_busy.compare_exchange_strong(expected, true)) // если _busy == true, то continue иначе меняет busy на true и идет дальше ифа
                continue;
            id = _id++; // следующий необработанный индекс будет возвращен а к самому индексу приавиться единица
            _busy = false; // делает _busy оратно false чтоб следующий поток мог взять необработанный индекс
            break;
        }
        return id;
    }
private:

    std::atomic_bool _busy;
};

//------------

class Mutex : public Counter {
public:
    Mutex(const int &dataSize) : Counter(dataSize) {}

    int next() override{
        int id;
        _mtx.lock(); // блокирует все остальные потоки
        id = _id++;
        _mtx.unlock(); // разблокирует все потоки
        return id;
    }

private:
    std::mutex _mtx;
};

//------------
const int size1 = 1024 * 1024; // размер массива
const int threads[] = {4, 8, 16, 32}; // количество потоков

void threadFunction(Counter *counter, int8_t *arr, const bool *sleep) { // функция которую выполняет поток
    while (counter->canContinue()) { // пока индекс меньше размера массива
        arr[counter->next()]++; // инкрементирует необработанный элемент
        if (*sleep) // если нужно усыплять поток
            std::this_thread::sleep_for(std::chrono::nanoseconds(10)); // усыпляет текущий поток на 10 наносекунд
    }
}

void setZeros(int8_t *arr) {
    for (int i = 0; i < size1; i++)
        arr[i] = 0;
}

bool check(const int8_t *arr) {
    for (int i = 0; i < size1; i++)
        if(arr[i] != 1)
            return false;
    return true;
}

void run(Counter &counter, int8_t *arr, const int &threadNum, const bool &sleep) {
    counter.reset();
    setZeros(arr);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(); // время начала выполнения
    std::thread *threads = new std::thread[threadNum];
    for (int i = 0; i < threadNum; i++) {
        threads[i] = std::thread(threadFunction, &counter, arr, &sleep); // инициирует массив потоков потоками с функцией для потоков внутри
    }
    for (int i = 0; i < threadNum; i++) {
        threads[i].join();  // ждет пока потоки не завершат работу
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();  // время конца выполнения
    if (sleep)
        std::cout << "With sleep:" << std::endl;
    else
        std::cout << "Without sleep:" << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(
            end - begin).count()  // расчет времени выполнения
              << " milliseconds. Check:" << (check(arr) ? "true" : "false")
              << "\n";  // проверяет массив на равномерное заполнения единицами
}

void task1() {
    Mutex mutex(size1); // создает счетчик с мьютексом с массивом размера size внутри
    Atomic atomic(size1); // создает атомарны счетчик с массивом размера size внутри
    int8_t *arr = new int8_t[size1];
    for (int i = 0; i < 4; i++) {
        std::cout << "================THREADS " << threads[i] << "================" << std::endl;
        std::cout << "--------Atomic--------" << std::endl;
        run(atomic, arr, threads[i], false);
        run(atomic, arr, threads[i], true);
        std::cout << "--------Mutex--------" << std::endl;
        run(mutex, arr, threads[i], false);
        run(mutex, arr, threads[i], true);
    }
}


//----------------task_2------------------------------------------------------------------------------------------------

static const int NumTasks = 4 * 1024 * 1024;
static const int NumProducers = 1;
static const int NumConsumers = 1;
static const int QueueSize = 4;
vector<thread> producers;
vector<thread> consumers;
atomic<int> Sum {0};
atomic<int> Counter {0};

template <class T>
void Consumer(T& q) {
    int consSum = 0;
    int consCounter = 0;
    uint8_t val = 0;
    while (true) {
        consCounter = consCounter;
        if (consCounter >= NumTasks * NumProducers) { break; }
        if (q.Pop(val)) { consSum += val; consCounter++; }
    }
    Sum.fetch_add(consSum);
}

template <class T>
void Producer(T& q) { for (int i = 0; i < NumTasks; i++) { q.Push(1); } }

class StaticQueue {
private:
    queue<uint8_t> queue;
    bool isFull() { return queue.size() >= QueueSize; }
    mutex mutx;
    condition_variable pushConditionVariable;
    condition_variable popConditionVariable;
public:
    void Push(uint8_t val) {
        unique_lock<mutex> uLocker(mutx);
        // поток становится в ожидание, если предикат ложен
        pushConditionVariable.wait(uLocker, [this] {return !isFull(); } );
        queue.push(val);
        popConditionVariable.notify_all();
    }
    bool Pop(uint8_t& val) {
        unique_lock<mutex> uLocker(mutx);
        if (queue.empty()) { popConditionVariable.wait_for(uLocker, chrono::milliseconds(1)); }
        if (!queue.empty()) { val = queue.front(); queue.pop(); pushConditionVariable.notify_all(); return true; }
        else { return false; }
    }
} st;

class DynamicQueue {
private:
    queue<uint8_t> queue;
    mutex mutx;
public:
    void Push(uint8_t value) { unique_lock<mutex> uLocker(mutx); queue.push(value); }
    bool Pop(uint8_t& val) {
        unique_lock<mutex> uLocker(mutx);
        if (queue.empty()) { uLocker.unlock(); this_thread::sleep_for(chrono::milliseconds(1)); uLocker.lock(); }
        if (!queue.empty()) { val = queue.front(); queue.pop(); return true; }
        else { return false; }
    }
} di;

void count_static_sum() {
    Sum = 0;
    Counter = 0;
    for (int i = 0; i < NumConsumers; i++) { consumers.push_back(thread(Consumer<StaticQueue>, ref(st))); }
    for (int i = 0; i < NumProducers; i++) { producers.push_back(thread(Producer<StaticQueue>, ref(st))); }

    auto start = chrono::system_clock::now();
    for (int i = 0; i < NumConsumers; i++) { consumers[i].join(); }
    for (int i = 0; i < NumProducers; i++) { producers[i].join(); }
    auto end = std::chrono::system_clock::now();

    cout << "Result by static: " << Sum << endl;

    cout << "Sum equals: ";
    if(Sum == NumProducers * NumTasks) cout << "true" << endl;
    else cout << "false" << endl;

    cout << "Time check: " << chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << endl;

    consumers.clear();
    producers.clear();
}

void count_dinamic_sum() {
    Sum = 0;
    Counter = 0;
    for (int i = 0; i < NumConsumers; i++) { consumers.push_back(thread(Consumer<DynamicQueue>, ref(di))); }
    for (int i = 0; i < NumProducers; i++) { producers.push_back(thread(Producer<DynamicQueue>, ref(di))); }

    auto start = chrono::system_clock::now();
    for (int i = 0; i < NumConsumers; i++) { consumers[i].join(); }
    for (int i = 0; i < NumProducers; i++) { producers[i].join(); }
    auto end = std::chrono::system_clock::now();

    cout << "Result by dinamic: " << Sum << endl;

    cout << "Sum equals: ";
    if(Sum == NumProducers * NumTasks) cout << "true" << endl;
    else cout << "false" << endl;

    cout << "Time check: " << chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << endl;

    consumers.clear();
    producers.clear();
}
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, const char * argv[])
{
    //------task_1------
    cout << "----------Task_1----------" << endl;
    task1();
    cout << endl;
    //------task_2------
    cout << "----------Task_2----------" << endl;
    count_static_sum();
    cout << endl;
    count_dinamic_sum();
    cout << "--------------------------" << endl;
    //-----------------
    return 0;
}
