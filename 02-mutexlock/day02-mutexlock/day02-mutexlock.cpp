// day02-mutexlock.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <mutex>
#include <thread>
#include <stack>

std::mutex  mtx1;// 用于保护共享数据的互斥锁
int shared_data = 100;// 共享数据示例

// 一个循环增加共享数据的线程函数，使用互斥锁来保护共享数据
void use_lock() {
    while (true) {
		mtx1.lock();// 加锁
		shared_data++;
		std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
		std::cout << "sharad data is " << shared_data << std::endl;
		mtx1.unlock();// 解锁
		std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

}

// 线程不安全的栈类模板
template<typename T>
class threadsafe_stack1
{
private:
	std::stack<T> data;// 数据栈
	mutable std::mutex m; // 保护数据栈的互斥锁
public:
	threadsafe_stack1() {} // 默认构造函数

	// 拷贝构造函数，确保线程安全
	threadsafe_stack1(const threadsafe_stack1& other)
	{
		// 互斥锁保护
		std::lock_guard<std::mutex> lock(other.m);
		//①在构造函数的函数体（constructor body）内进行复制操作
		data = other.data;
	}
	// 禁止赋值操作
	threadsafe_stack1& operator=(const threadsafe_stack1&) = delete;
	// 添加元素
	void push(T new_value)
	{
		std::lock_guard<std::mutex> lock(m);
		data.push(std::move(new_value));
	}

	// 弹出元素（问题：返回拷贝的栈顶元素）
	T pop()
	{
		std::lock_guard<std::mutex> lock(m);
		auto element = data.top();
		data.pop();
		return element;
	}
	// 检查栈是否为空
	bool empty() const
	{
		std::lock_guard<std::mutex> lock(m);
		return data.empty();
	}
};
// 测试线程不安全栈
void test_threadsafe_stack1() {
	threadsafe_stack1<int> safe_stack;
	safe_stack.push(1);

	std::thread t1([&safe_stack]() {
		if (!safe_stack.empty()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			safe_stack.pop();
			}
		});

	std::thread t2([&safe_stack]() {
		if (!safe_stack.empty()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			safe_stack.pop();
		}
	});

	t1.join();
	t2.join();
}

// 自定义异常类，用于空栈的异常处理
struct empty_stack : std::exception
{
	const char* what() const throw()
	{
		return "Stack is empty";
	}
};

// 改进的线程安全栈，提供异常安全的 pop 方法
template<typename T>
class threadsafe_stack
{
private:
	std::stack<T> data;
	mutable std::mutex m;
public:
	threadsafe_stack() {}
	threadsafe_stack(const threadsafe_stack& other)
	{
		std::lock_guard<std::mutex> lock(other.m);
		//①在构造函数的函数体（constructor body）内进行复制操作
		data = other.data;   
	}
	threadsafe_stack& operator=(const threadsafe_stack&) = delete;
	void push(T new_value)
	{
		std::lock_guard<std::mutex> lock(m);
		data.push(std::move(new_value));
	}
	// 弹出元素并返回 shared_ptr 指针，这里不一样
	std::shared_ptr<T> pop()
	{
		std::lock_guard<std::mutex> lock(m);
		//②试图弹出前检查是否为空栈
		if (data.empty()) throw empty_stack();
		//③改动栈容器前设置返回值
			std::shared_ptr<T> const res(std::make_shared<T>(data.top()));    
			data.pop();
		return res;
	}
	// 弹出元素并存储在传入的引用中
	void pop(T& value)
	{
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) throw empty_stack();
		value = data.top();
		data.pop();
	}
	bool empty() const
	{
		std::lock_guard<std::mutex> lock(m);
		return data.empty();
	}
};
// 测试锁和共享数据
void test_lock() {
	std::thread t1(use_lock);

	std::thread t2([]() {
		while (true) {
			mtx1.lock();
			shared_data--;
			std::cout << "current thread is " << std::this_thread::get_id() << std::endl;
			std::cout << "sharad data is " << shared_data << std::endl;
			mtx1.unlock();
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
		});

	t1.join();
	t2.join();
}

std::mutex  t_lock1;
std::mutex  t_lock2;
int m_1 = 0;
int m_2 = 1;

// 演示死锁：不同线程按不同顺序加锁
void dead_lock1() {
	while (true) {
		std::cout << "dead_lock1 begin " << std::endl;
		t_lock1.lock();
		m_1 = 1024;
		t_lock2.lock();
		m_2 = 2048;
		t_lock2.unlock();
		t_lock1.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		std::cout << "dead_lock1 end " << std::endl;
	}
}

void dead_lock2() {
	while (true) {
		std::cout << "dead_lock2 begin " << std::endl;
		t_lock2.lock();
		m_2 = 2048;
		t_lock1.lock();
		m_1 = 1024;
		t_lock1.unlock();
		t_lock2.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		std::cout << "dead_lock2 end " << std::endl;
	}
}


void atomic_lock1() {
	std::cout << "lock1 begin lock" << std::endl;
	t_lock1.lock();
	m_1 = 1024;
	t_lock1.unlock();
	std::cout << "lock1 end lock" << std::endl;
}

void atomic_lock2() {
	std::cout << "lock2 begin lock" << std::endl;
	t_lock2.lock();
	m_2 = 2048;
	t_lock2.unlock();
	std::cout << "lock2 end lock" << std::endl;
}

void safe_lock1() {
	while (true) {
		atomic_lock1();
		atomic_lock2();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void safe_lock2() {
	while (true) {
		atomic_lock2();
		atomic_lock1();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void test_dead_lock() {
	std::thread t1(dead_lock1);
	std::thread t2(dead_lock2);
	t1.join();
	t2.join();
}

void test_safe_lock() {
	std::thread t1(safe_lock1);
	std::thread t2(safe_lock2);
	t1.join();
	t2.join();
}

//对于要使用两个互斥量，可以同时加锁，如不同时加锁可能会存在问题

//假设这是一个很复杂的数据结构, 假设不建议拷贝操作
class som_big_object {
public:
	som_big_object(int data) :_data(data) {}
	//拷贝构造
	som_big_object(const som_big_object& b2) :_data(b2._data) {
		_data = b2._data;
	}
	//移动构造
	som_big_object(som_big_object&& b2) :_data(std::move(b2._data)) {

	}
	//重载输出运算符
	friend std::ostream& operator << (std::ostream& os, const som_big_object& big_obj) {
		os << big_obj._data;
		return os;
	}

	//重载赋值运算符
	som_big_object& operator = (const som_big_object& b2) {
		if (this == &b2) {
			return *this;
		}
		_data = b2._data;
		return *this;
	}

	//交换数据
	friend void swap(som_big_object& b1, som_big_object& b2) {
		som_big_object temp = std::move(b1);
		b1 = std::move(b2);
		b2 = std::move(temp);
	}
private:
	int _data;
};

//假设这是一个结构包含了锁与复杂的成员对象
class big_object_mgr {
public:
	big_object_mgr(int data = 0) :_obj(data) {}
	void printinfo() {
		std::cout << "current obj data is " << _obj << std::endl;
	}
	friend void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2);
	friend void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2);
	friend void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2);
private:
	std::mutex _mtx;
	som_big_object _obj;
};

void danger_swap(big_object_mgr& objm1, big_object_mgr& objm2) {
	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&objm1 == &objm2) {
		return;
	}
	std::lock_guard <std::mutex> gurad1(objm1._mtx);
	//此处为了故意制造死锁，我们让线程小睡一会
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard<std::mutex> guard2(objm2._mtx);
	swap(objm1._obj, objm2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

//test danger_swap
void  test_danger_swap() {
	big_object_mgr objm1(5);
	big_object_mgr objm2(100);

	std::thread t1(danger_swap, std::ref(objm1), std::ref(objm2));
	std::thread t2(danger_swap, std::ref(objm2), std::ref(objm1));
	t1.join();
	t2.join();

	objm1.printinfo();
	objm2.printinfo();
}

//更安全的方式一下锁住要用的多个互斥量
void safe_swap(big_object_mgr& objm1, big_object_mgr& objm2) {
	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&objm1 == &objm2) {
		return;
	}

	//更改处同时加锁！
	std::lock(objm1._mtx, objm2._mtx);
	//领养锁管理它自动释放
	std::lock_guard <std::mutex> gurad1(objm1._mtx, std::adopt_lock);

	//此处为了故意制造死锁，我们让线程小睡一会
	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::lock_guard <std::mutex> gurad2(objm2._mtx, std::adopt_lock);

	swap(objm1._obj, objm2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

//上述代码可以简化为以下方式
void safe_swap_scope(big_object_mgr& objm1, big_object_mgr& objm2) {
	std::cout << "thread [ " << std::this_thread::get_id() << " ] begin" << std::endl;
	if (&objm1 == &objm2) {
		return;
	}

	std::scoped_lock  guard(objm1._mtx, objm2._mtx);
	//等价于
	//std::scoped_lock<std::mutex, std::mutex> guard(objm1._mtx, objm2._mtx);
	swap(objm1._obj, objm2._obj);
	std::cout << "thread [ " << std::this_thread::get_id() << " ] end" << std::endl;
}

void test_safe_swap() {
	big_object_mgr objm1(5);
	big_object_mgr objm2(100);

	std::thread t1(safe_swap, std::ref(objm1), std::ref(objm2));
	std::thread t2(safe_swap, std::ref(objm2), std::ref(objm1));
	t1.join();
	t2.join();

	objm1.printinfo();
	objm2.printinfo();
}


void test_safe_swap_scope() {
	big_object_mgr objm1(5);
	big_object_mgr objm2(100);

	std::thread t1(safe_swap_scope, std::ref(objm1), std::ref(objm2));
	std::thread t2(safe_swap_scope, std::ref(objm2), std::ref(objm1));
	t1.join();
	t2.join();

	objm1.printinfo();
	objm2.printinfo();
}
//对于现实开发中，我们很难保证嵌套加锁，所以尽可能将互斥操作封装为原子操作，尽量不要在一个函数里嵌套用两个锁。
//对于嵌套用锁，也可以采用权重的方式限制使用顺序。

// 层级锁
class hierarchical_mutex {
public:
	explicit hierarchical_mutex(unsigned long value) : _hierarchy_value(value), _previous_hierarchy_value(0) {}
	hierarchical_mutex(const hierarchical_mutex&) = delete;
	hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;

	void lock() {
		check_for_hierarchy_violation();
		_internal_mutex.lock();
		update_hierarchy_value();
	}

	void unlock() {
		if (_this_thread_hierarchy_value != _hierarchy_value) {
			throw std::logic_error("mutex hierarchy violated");
		}
		_this_thread_hierarchy_value = _previous_hierarchy_value;
		_internal_mutex.unlock();
	}

	bool try_lock() {
		check_for_hierarchy_violation();
		if (!_internal_mutex.try_lock()) {  // 修正条件判断
			return false;
		}
		update_hierarchy_value();
		return true;
	}

private:
	std::mutex _internal_mutex;
	unsigned long const _hierarchy_value;
	unsigned long _previous_hierarchy_value;
	static thread_local unsigned long _this_thread_hierarchy_value;

	void check_for_hierarchy_violation() {
		if (_this_thread_hierarchy_value <= _hierarchy_value) {
			throw std::logic_error("mutex hierarchy violated");
		}
	}

	void update_hierarchy_value() {
		_previous_hierarchy_value = _this_thread_hierarchy_value;
		_this_thread_hierarchy_value = _hierarchy_value;
	}
};

thread_local unsigned long hierarchical_mutex::_this_thread_hierarchy_value(ULONG_MAX);

void test_hierarchy_lock() {
	hierarchical_mutex hmtx1(1000);
	hierarchical_mutex hmtx2(500);
	try {
		std::thread t1([&hmtx1, &hmtx2]() {
			try {
				hmtx1.lock();
				hmtx2.lock();
				hmtx2.unlock();
				hmtx1.unlock();
			}
			catch (const std::exception& e) {
				std::cerr << "Thread 1 Exception: " << e.what() << std::endl;
			}
			});

		std::thread t2([&hmtx1, &hmtx2]() {
			try {
				hmtx2.lock();
				hmtx1.lock();
				hmtx1.unlock();
				hmtx2.unlock();
			}
			catch (const std::exception& e) {
				std::cerr << "Thread 2 Exception: " << e.what() << std::endl;
			}
			});

		t1.join();
		t2.join();
	}
	catch (const std::logic_error& e) {
		std::cerr << "Logic error: " << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}



int main()
{
	//use_lock();

	//test_threadsafe_stack1();

	//test_lock();

	//test_dead_lock();

	//test_safe_lock();
    
	//test_danger_swap();
	
	//test_safe_swap();

	//test_safe_swap_scope();

	test_hierarchy_lock();

	std::cout << "Hello World!\n";
	system("pause");
}


