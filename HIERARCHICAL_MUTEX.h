#pragma once
#include<mutex>
class hierarchical_mutex {
	std::mutex internal_mutex;

	unsigned long const hierarchy_value;
	unsigned long previous_hierarchy_value;
	/**
	 * this_thread_hierarchy_value使用thread_local的值来代表当前线程的层级
	 * 声明thread_local，每个线程都有其拷贝副本，线程中变量状态完全独立，
	 * 当从另一个线程进行读取时，变量的状态也完全独立
	*/
	static thread_local unsigned long this_thread_hierarchy_value;

	void check_for_hierarchy_violation() {
		if (this_thread_hierarchy_value <= hierarchy_value) {
			throw std::logic_error("mutex hierarchy violated");
		}
	}

	void update_hierarchy_value() {
		previous_hierarchy_value = this_thread_hierarchy_value;
		this_thread_hierarchy_value = hierarchy_value;
	}

public:
	explicit hierarchical_mutex(unsigned long value):hierarchy_value(value) {
		
		previous_hierarchy_value = 0;
	}

	void lock() {
		check_for_hierarchy_violation();
		internal_mutex.lock();

		update_hierarchy_value();
	}

	void unlock() {
		this_thread_hierarchy_value = previous_hierarchy_value;
		internal_mutex.lock();
	}

	bool try_lock() {
		check_for_hierarchy_violation();
		if (!internal_mutex.try_lock()) 
			return false;
		update_hierarchy_value();
		return true;
	}

};
//初始化为ULONG_MAX
thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);