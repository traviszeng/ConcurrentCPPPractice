#include<iostream>
using namespace std;
/**
使用lock_guard用互斥量来进行互斥操作
*/

/*使用互斥量保护列表*/
/**
互斥量放在类中和需要保护的数据同时声明为private
当所有成员函数都会在调用时对数据上锁，结束时对数据解锁，那么就保证了数据访问时不变量不被破坏。

但存在问题：
当其中一个成员函数返回的是保护数
据的指针或引用时，会破坏对数据的保护。具有访问能力的指针或引用可以访问(并可能修改)
被保护的数据，而不会被互斥锁限制。互斥量保护的数据需要对接口的设计相当谨慎，要确
保互斥量能锁住任何对保护数据的访问，并且不留后门。
*/
#include<mutex>
#include<list>
#include<algorithm>

std::list<int> somelist;
std::mutex some_mutex;

void add_to_list(int new_value) {
	std::lock_guard<mutex> guard(some_mutex);
	somelist.push_back(new_value);
}

bool list_contains(int value_to_find) {
	std::lock_guard<mutex> guard(some_mutex);
	return std::find(somelist.begin(), somelist.end(), value_to_find) != somelist.end();
}

/**
将保护数据作为一个运行时参数,无意中传递了保护数据的引用
*/
/**
切勿将受保护数据的指针或引用传递到互斥锁作用域之外，无论
是函数返回值，还是存储在外部可见内存，亦或是以参数的形式传递到用户提供的函数中
去。
*/

class some_data
{
	int a;
	std::string b;
public:
	void do_something();
};

class data_wrapper
{
private:
	some_data data;
	std::mutex m;
public:
	template<typename Function>
	void process_data(Function func)
	{
		std::lock_guard<std::mutex> l(m);
		func(data); // 1 传递“保护”数据给用户函数
	}
};
some_data* unprotected;
void malicious_function(some_data& protected_data)
{
	unprotected = &protected_data;
}
data_wrapper x;
void foo()
{
	x.process_data(malicious_function); // 2 传递一个恶意函数
	unprotected->do_something(); // 3 在无保护的情况下访问保护数据
}

/**线程安全的栈*/
#include "threadsafe_stack.h"

/**使用std::lock解决死锁*/
// 这里的std::lock()需要包含<mutex>头文件
class some_big_object {
	some_big_object();
};
void swap(some_big_object& lhs, some_big_object& rhs);
class X
{
private:
	some_big_object some_detail;
	std::mutex m;
public:
	X(some_big_object const& sd) :some_detail(sd) {}
	friend void swap(X& lhs, X& rhs)
	{
		if (&lhs == &rhs)
			return;
		std::lock(lhs.m, rhs.m); // 用std::lock锁住两个互斥量，两个互斥量要么全部锁住，要么一个都不锁
		std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock); // 提供 std::adopt_lock 参数除了表示 std::lock_guard 对象已经上锁外，还表示现成的锁，而非尝试创建新的锁。
		std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock); 
		swap(lhs.some_detail, rhs.some_detail);
	}
};

/**使用层次锁来避免死锁*/
#include"HIERARCHICAL_MUTEX.h"
hierarchical_mutex high_level_mutex(10000); // 1
hierarchical_mutex low_level_mutex(5000); // 2
int do_low_level_stuff();
int low_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex); // 3
	return do_low_level_stuff();
}
void high_level_stuff(int some_param);
void high_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex); // 4
	high_level_stuff(low_level_func()); // 5
}
void thread_a() // 6
{
	high_level_func();
}
hierarchical_mutex other_mutex(100); // 7
void do_other_stuff();
void other_stuff()
{
	high_level_func(); // 8
	do_other_stuff();
}
void thread_b() // 9
{
	std::lock_guard<hierarchical_mutex> lk(other_mutex); // 10
	other_stuff();
}