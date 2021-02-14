#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <chrono>

using namespace std;

//----------------task_1------------------------------------------------------------------------------------------------

const int size_of_byte_array = 1024 * 1024;
const int threads[] = { 4, 8, 16, 32 };
vector<int8_t> byte_array(size_of_byte_array);

class Counter {
protected:
    int _id;
    int _size;
public:
    Counter(const int& size_of_byte_array) { _id = 0; _size = size_of_byte_array; }
    void set_zero_id() { _id = 0; }
    bool check_for_continuation() { return _id < _size; }
    virtual int swap_to_next_id() = 0;
};

class Atomic : public Counter {
private:
    atomic_int _indexer = 0;
public:
    void set_zero_id() { _indexer = 0; }
    Atomic(const int& size_of_byte_array) : Counter(size_of_byte_array) {}
    int swap_to_next_id() override {
        return _indexer++;
    }
};

class Mutex : public Counter {
private:
    std::mutex _mutx;
public:
    Mutex(const int& size_of_byte_array) : Counter(size_of_byte_array) {}
    int swap_to_next_id() override { int id; _mutx.lock(); id = _id; _id++; _mutx.unlock(); return id; }
};

void initialize_array_with_zeros(vector<int8_t>& _array_of_bytes) {
    for (int i = 0; i < size_of_byte_array; i++) _array_of_bytes.at(i) = 0;
}

bool result_array_cheking(const vector<int8_t>& result_array_of_bytes) {
    for (int i = 0; i < size_of_byte_array; i++)
        if ((int)result_array_of_bytes.at(i) != 1) return false;
    return true;
}

void exucutable_func(Counter* counter, vector<int8_t>& array_of_bytes, const bool sleep) {
    int indexer;
    do {
        indexer = counter->swap_to_next_id();
        if (indexer >= size_of_byte_array) break;
        array_of_bytes.at(indexer) += 1;
        if (sleep) this_thread::sleep_for(chrono::nanoseconds(10));
    } while (1);
}

/*
void exucutable_func(Counter* counter, vector<int8_t>& array_of_bytes, const bool sleep)
{
    while (counter->check_for_continuation()) {
        //_array_of_bytes[counter->swap_to_next_id()]++;
        array_of_bytes.at(counter->swap_to_next_id())++;
        long temp = (long)counter->swap_to_next_id();
        array_of_bytes.at(temp) += 1;
        printf("index = %d data = %d \n", temp, array_of_bytes.at(temp));
        if (sleep) this_thread::sleep_for(chrono::nanoseconds(10));
    }
}
*/

void run(Counter& counter, vector<int8_t>& array_of_bytes, const int& threadNum, const bool sleep) {
    counter.set_zero_id();
    initialize_array_with_zeros(array_of_bytes);
    
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    thread* threads = new thread[threadNum];

    //thread test(exucutable_func, &counter, ref(array_of_bytes), sleep);
    //thread test1(exucutable_func, &counter, ref(array_of_bytes), sleep);
    //test.join();
    //test1.join();
    
    for (int i = 0; i < threadNum; i++) { threads[i] = thread(exucutable_func, &counter, ref(array_of_bytes), sleep); }
    for (int i = 0; i < threadNum; i++) { threads[i].join(); }
    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "Time of execution: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
    cout << "Is true result: " << (result_array_cheking(array_of_bytes) ? "true" : "false") << "\n";
}

void print_theads_result() {
    //int8_t* byte_array = new int8_t[size_of_byte_array];
    //vector<int8_t> byte_array(size_of_byte_array);
    Atomic atomic_counter(size_of_byte_array);
    Mutex mutex_counter(size_of_byte_array);
    for (int i = 0; i < 4; i++) {
        cout << "--------------------------" << endl;
        cout << "Number of threads: " << threads[i] << endl;
        cout << "--------------------------" << endl;
        cout << "Using atomic: " << endl << endl;
        run(atomic_counter, byte_array, threads[i], false);
        cout << endl << "Using thread sleeping: " << endl;
        //initialize_array_with_zeros(byte_array);
        atomic_counter.set_zero_id();
        run(atomic_counter, byte_array, threads[i], true);
        atomic_counter.set_zero_id();
        cout << "--------------------------";
        cout << endl << "Using mutex: " << endl << endl;
        run(mutex_counter, byte_array, threads[i], false);
        cout << endl << "With sleep:" << endl;
        run(mutex_counter, byte_array, threads[i], true);
        cout << endl;
    }
}

//----------------task_2------------------------------------------------------------------------------------------------

static const int NumTasks = 4 * 1024 * 1024;
static const int NumProducers = 1;
static const int NumConsumers = 1;
static const int QueueSize = 4;
vector<thread> producers;
vector<thread> consumers;
atomic<int> Sum{ 0 };
atomic<int> Counter{ 0 };

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
        pushConditionVariable.wait(uLocker, [this] {return !isFull(); });
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
    bool Pop(uint8_t& value) {
        unique_lock<mutex> uLocker(mutx);
        if (queue.empty()) { uLocker.unlock(); this_thread::sleep_for(chrono::milliseconds(1)); uLocker.lock(); }
        if (!queue.empty()) { value = queue.front(); queue.pop(); return true; }
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
    if (Sum == NumProducers * NumTasks) cout << "true" << endl;
    else cout << "false" << endl;

    cout << "Time of execution: " << chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << endl;

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
    if (Sum == NumProducers * NumTasks) cout << "true" << endl;
    else cout << "false" << endl;

    cout << "Time of execution: " << chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << endl;

    consumers.clear();
    producers.clear();
}
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
    //------task_1------
    cout << "----------Task_1----------" << endl << endl;
    print_theads_result();
    //------task_2------
    cout << "----------Task_2----------" << endl << endl;
    count_static_sum();
    cout << endl;
    count_dinamic_sum();
    cout << endl << "--------------------------" << endl;
    //-----------------
    return 0;
}
