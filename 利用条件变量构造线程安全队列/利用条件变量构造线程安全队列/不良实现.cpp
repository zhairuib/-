#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mtx_num;
int num = 1;  // 初始值为1，以便线程A先打印

void PoorImpleman() {
    std::thread t1([]() {
        for (;;) {
            {
                std::lock_guard<std::mutex> lock(mtx_num);
                if (num == 1) {
                    std::cout << "Thread A print 1....." << std::endl;
                    num++;
                    continue;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        });

    std::thread t2([]() {
        for (;;) {
            {
                std::lock_guard<std::mutex> lock(mtx_num);
                if (num == 2) {
                    std::cout << "Thread B print 2....." << std::endl;
                    num--;
                    continue;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        });

    t1.join();
    t2.join();
}

int main() {
    PoorImpleman();
    return 0;
}
