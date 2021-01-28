#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <chrono>

using namespace std;

//----------------task_1------------------------------------------------------------------------------------------------



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
