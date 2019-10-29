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
	void do_something() {}
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
/**
 * 底层的获取后高层的无法加锁
*/
hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

int do_low_level_stuff() {
	return 0;
}

int low_level_func(){
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
	return do_low_level_stuff();
}

void high_level_stuff(int param) {}

void high_level_func(){
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
	high_level_stuff(low_level_func());
}

void thread_a(){
	high_level_func();
}

hierarchical_mutex other_mutex(100);
void do_other_stuff() {}

f7.wait()
{
	return f7 .();
}

void other_stuff(){
	high_level_func();
	do_other_stuff();
}

void thread_b(){ //exception 
	std::lock_guard<hierarchical_mutex> lk(other_mutex);
	other_stuff();
}

//使用 unique_lock重写swap
void swap(some_big_object &l, some_big_object & r) {}
class X_unique{
private:
	some_big_object detail;
	std::mutex m;
public:
	X_unique(some_big_object const& sd):detail(sd){}
	friend void swap(X_unique& l,X_unique& r){
		if(&l==&r){
			return ;
		}
		std::unique_lock<std::mutex> lock_a(l.m,std::defer_lock);
		std::unique_lock<std::mutex> lock_b(r.m,std::defer_lock);
		std::lock(lock_a,lock_b);
		swap(r.detail,l.detail);
	}
};

//std::unique_lock是可移动的，但不可赋值的类型。
std::unique_lock<mutex> get_lock() {
	extern std::mutex some_mutex;
	std::unique_lock<mutex> lk(some_mutex);
	//prepare_data();
	return lk;
}

void process_data() {
	unique_lock<mutex> lk(get_lock());  //编译器自动调用移动构造函数
	//do_something();
}

void get_and_process_data() {
	mutex mu;
	unique_lock<mutex> my_lock(mu);
	//some_class data_to_process = get_next_data_chunk();
	my_lock.unlock(); // 1 不要让锁住的互斥量越过process()函数的调用
	//result_type result = process(data_to_process);
	my_lock.lock(); // 2 为了写入数据，对互斥量再次上锁
	//write_result(data_to_process, result);
}

//比较操作符中一次锁住一个互斥量
class Y
{
private:
	int some_detail;
	mutable std::mutex m;
	int get_detail() const
	{
		std::lock_guard<std::mutex> lock_a(m); // 1
		return some_detail;
	}
public:
	Y(int sd) :some_detail(sd) {}
	friend bool operator==(Y const& lhs, Y const& rhs)
	{
		if (&lhs == &rhs)
			return true;
		int const lhs_value = lhs.get_detail(); // 2
		int const rhs_value = rhs.get_detail(); // 3
		return lhs_value == rhs_value; // 4
	}
};

//可能在2 3后 左右值会被修改，此时只是保证==为某一时刻的状态

/**
保护共享数据的初始化过程
*/

//延迟初始化：每一个操作都需要先对源进行检查，
//为了了解数据是否被初始化，然后在其使用前决定，数据是否需要初始化
//单线程版本
shared_ptr<some_data> resource_ptr;
void lazyInit() {
	if (!resource_ptr) {
		resource_ptr.reset(new some_data);//转为多线程时需要保护，这是因为每个线程必须等待互斥量，为了确定数据源已经初始化了。
	}
	resource_ptr->do_something;
}

//使用一个互斥量的延迟初始化(线程安全)过程
std::mutex resource_mutex;
void lazyInit()
{
	std::unique_lock<std::mutex> lk(resource_mutex); // 所有线程在此序列化
	if (!resource_ptr)
	{
		resource_ptr.reset(new some_data); // 只有初始化过程需要保护
	}
	lk.unlock();
	resource_ptr->do_something();
}

//双重检查锁模式
/**
外部的读取锁①没有与内部的
写入锁进行同步③。因此就会产生条件竞争
*/
void undefined_behaviour_with_double_checked_locking()
{
	if (!resource_ptr) // 1
	{
		std::lock_guard<std::mutex> lk(resource_mutex);
		if (!resource_ptr) // 2
		{
			resource_ptr.reset(new some_data); // 3
		}
	}
	resource_ptr->do_something(); // 4
}


/*
使用std::call_once和std::once_flag来处理
比起锁住互斥量，并显式的检查指针，每个线程只需要使用 std::call_once 
在 std::call_once 的结束时，就能安全的知道指针已经被其他的线程初始化了。
*/
std::once_flag resource_flag;
void init_resource() {
	resource_ptr.reset(new some_data);
}

void init_with_call_once() {
	std::call_once(resource_flag, init_resource); //完整进行一次初始化
	resource_ptr->do_something();
}

// 使用 std::call_once 作为类成员的延迟初始化(线程安全)
/**
第一个调用send_data()①或receive_data()③的线程完成初始化过程。
使用成员函数open_connection()去初始化数据
*/
class X
{
private:
	connection_info connection_details;
	connection_handle connection;
	std::once_flag connection_init_flag;
	void open_connection()
	{
		connection = connection_manager.open(connection_details);
	}
public:
	X(connection_info const& connection_details_) :
		connection_details(connection_details_)
	{}
	void send_data(data_packet const& data) // 1
	{
		std::call_once(connection_init_flag, &X::open_connection, this); // 2
		connection.send_data(data);
	}
	data_packet receive_data() // 3
	{
		std::call_once(connection_init_flag, &X::open_connection, this); // 2
		return connection.receive_data();
	}
};
/**
用 boost::shared_lock<boost::shared_mutex> 获取共享访问权。这与使用 std::unique_lock 一
样，除非多线要在同时得到同一个 boost::shared_mutex 上有共享锁。唯一的限制就是，当任
一线程拥有一个共享锁时，这个线程就会尝试获取一个独占锁，直到其他线程放弃他们的
锁；同样的，当任一线程拥有一个独占锁是，其他线程就无法获得共享锁或独占锁，直到第
一个线程放弃其拥有的锁。
*/
// 使用 boost::shared_mutex 对数据结构进行保护
#include <map>
#include <string>
#include <mutex>

#include <boost/thread.hpp>    
#include <boost/thread/mutex.hpp>

class dns_entry {};

class dns_cache {
	std::map<string, dns_entry> entries;
	mutable boost::shared_mutex entry_mutex;
public:
	/**使用了 boost::shared_lock<> 实例来保护其共享和只读权限①；
	这就使得，多线程可以同时调用find_entry()，且不会出错*/
	dns_entry find_entry(string const& domain) const {
		boost::shared_lock<boost::shared_mutex> lk(entry_mutex); //1
		std::map<string, dns_entry>::const_iterator  const it = entries.find(domain);
		return (it == entries.end()) ? dns_entry() : it->second;
	}
	/**当表格需要更新时②，为其提供独占访问权限；
	在update_or_add_entry()函数调用时，独占锁会阻止其他线程对数据结构进行修改，并且这些线程在这时，也不能调用find_entry()。*/
	void update_or_add_entry(std::string const& domain,dns_entry const& dns_details)
	{
		std::lock_guard<boost::shared_mutex> lk(entry_mutex); // 2
		entries[domain] = dns_details;
	}

};