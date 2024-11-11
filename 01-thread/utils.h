#pragma once
// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <thread>
#include <string>
#include <memory>
#include <iostream>

struct func {
    int& _i;
    func(int& i) : _i(i) {} // 构造函数
    void operator()() {
        for (int i = 0; i < 3; i++) {
            _i = i; // 通过引用修改外部的值
            std::cout << "_i is " << _i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟耗时操作
        }
    }
};

class background_task {
public:
    void operator()() {
        std::cout << "background_task called" << std::endl;
    }
};

// thread_examples.cpp
void print_str(int i, const std::string& s);
void change_param(int& param);
void thead_work1(std::string str);
void oops();
void use_join();
void catch_exception();
void danger_oops(int som_param);
void safe_oops(int some_param);
void ref_oops(int some_param);
void day01();

void auto_guard();



// parallel_accumulate.cpp
void use_parallel_acc();

// day02 函数声明
void day02();
#endif
