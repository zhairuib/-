// thread_examples.cpp
#include "utils.h"
#include "joining_thread.h"
#include <thread>
#include <chrono>

// print_str 函数定义
void print_str(int i, const std::string& s) {
    std::cout << "i is " << i << " str is " << s << std::endl;
}


// 简单的线程函数，接受一个字符串并输出
void thead_work1(std::string str) {
    std::cout << "str is " << str << std::endl;
}



// 示例函数 oops：通过 detach 方式创建线程，演示隐患
// func 对象捕获了局部变量，detach 后主线程可能在局部变量销毁前结束
void oops() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    functhread.detach();
}

// 安全的线程创建示例，使用 join 等待线程完成，避免局部变量访问错误
void use_join() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread(myfunc);
    functhread.join();
}


// 捕获异常的函数示例
void catch_exception() {
    int some_local_state = 0;
    func myfunc(some_local_state);
    std::thread functhread{ myfunc };
    try {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (std::exception& e) {
        functhread.join();
        throw;
    }
    functhread.join();
}

// 错误示例：detach 后访问局部变量，可能导致崩溃
void danger_oops(int som_param) {
    char buffer[1024];
    sprintf_s(buffer, "%i", som_param);
    std::thread t(print_str, 3, buffer);
    t.detach();
    std::cout << "danger oops finished " << std::endl;
}

// 安全示例：在创建线程前转换为 std::string 确保线程安全
void safe_oops(int some_param) {
    char buffer[1024];
    sprintf_s(buffer, "%i", some_param);
    std::thread t(print_str, 3, std::string(buffer));
    t.detach();
}

void change_param(int& param) {
    param++;
}
////当线程要调用的回调函数参数为引用类型时，需要将参数显示转化为引用对象传递给线程的构造函数， 如果采用如下调用会编译失败
//void ref_oops(int some_param) {
//    std::cout << "before change , param is " << some_param << std::endl;
//    //需使用引用显示转换
//    std::thread  t2(change_param, some_param);
//    t2.join();
//    std::cout << "after change , param is " << some_param << std::endl;
//}

// 使用 std::ref 显式传递引用，避免副本传递
void ref_oops(int some_param) {
    std::cout << "before change , param is " << some_param << std::endl;
    std::thread t2(change_param, std::ref(some_param));
    t2.join();
    std::cout << "after change , param is " << some_param << std::endl;
}

class thread_guard {
private:
    std::thread& _t;
public:
    explicit thread_guard(std::thread& t) :_t(t) {}
    ~thread_guard() {
        //join只能调用一次
        if (_t.joinable()) {
            _t.join();
        }
    }

    thread_guard(thread_guard const&) = delete;
    thread_guard& operator=(thread_guard const&) = delete;
};

void auto_guard() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread  t(my_func);
    thread_guard g(t);
    //本线程做一些事情
    std::cout << "auto guard finished " << std::endl;
}

void deal_unique(std::unique_ptr<int> p) {
    std::cout << "unique ptr data is " << *p << std::endl;
    (*p)++;

    std::cout << "after unique ptr data is " << *p << std::endl;
}

void move_oops() {
    auto p = std::make_unique<int>(100);
    std::thread  t(deal_unique, std::move(p));
    t.join();
    //不能再使用p了，p已经被move废弃
   // std::cout << "after unique ptr data is " << *p << std::endl;
}

void day01() {
    //std::string hellostr = "hello world!";
    //std::thread t1(thead_work1, hellostr);
    //t1.join();

    //std::thread t2((background_task()));
    //t2.join();

    //std::thread t3{ background_task() };
    //t3.join();

    //std::string hellostr = "hello world!";
    //std::thread t4([](std::string  str) {
    //    std::cout << "str is " << str << std::endl;
    //    }, hellostr);
    //t4.join();
    
    //如果是绑定类的成员函数，必须添加 &
    //std::thread t2(&thead_work1, hellostr);

    move_oops();

    //ref_oops(10);

    //oops();
    //std::this_thread::sleep_for(std::chrono::seconds(1));
   
    //use_join();
    
    //catch_exception();

   // auto_guard();

    //danger_oops(100);
    //std::this_thread::sleep_for(std::chrono::seconds(2));

    // safe_oops(100);
    //std::this_thread::sleep_for(std::chrono::seconds(2));


    system("pause");
}
