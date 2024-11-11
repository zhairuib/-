// parallel_accumulate.cpp
#include "utils.h"
#include <vector>
#include <numeric>
#include <thread>

void some_function() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// 示例函数，持续运行以便演示线程转移
void some_other_function() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// 线程的移动和转移管理示例
void dangerous_use() {
    std::thread t1(some_function);
    std::thread t2 = std::move(t1); // 转移 t1 到 t2，t1 无效
    t1 = std::thread(some_other_function); // t1 绑定新线程
    std::thread t3;
    t3 = std::move(t2); // 将 t2 转移给 t3
    t1 = std::move(t3); // 再次转移管理权
    std::this_thread::sleep_for(std::chrono::seconds(2000));
}

// joining_thread 类，线程包装器，确保析构时自动 join
class joining_thread {
    std::thread  _t;
public:
    joining_thread() noexcept = default;

    template<typename Callable, typename ... Args>
    explicit  joining_thread(Callable&& func, Args&& ...args) :
        _t(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    explicit joining_thread(std::thread  t) noexcept : _t(std::move(t)) {}

    ~joining_thread() noexcept {
        if (joinable()) {
            join();
        }
    }

    bool joinable() const noexcept {
        return _t.joinable();
    }

    void join() {
        _t.join();
    }
};

// 使用 joining_thread 的示例函数
void use_jointhread() {
    joining_thread j1([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10);
}


template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread([=, &results]() {
            results[i] = std::accumulate(block_start, block_end, 0);
            });
        block_start = block_end;
    }
    std::accumulate(block_start, last, results[num_threads - 1]);

    for (auto& entry : threads) entry.join();
    return std::accumulate(results.begin(), results.end(), init);
}

void use_parallel_acc() {
    std::vector<int> vec(10000);
    std::iota(vec.begin(), vec.end(), 0);
    int sum = parallel_accumulate(vec.begin(), vec.end(), 0);
    std::cout << "sum is " << sum << std::endl;
}

// day02 函数的实现，调用并行累加功能
void day02() {
    //dangerous_use();
    use_jointhread();
    //use_parallel_acc();
}