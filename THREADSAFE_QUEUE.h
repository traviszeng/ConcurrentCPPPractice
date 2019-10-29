#pragma once
/**
1. 对整个队列的状态进行查询(empty()和size());
2.查询在队列中的各个元素(front()和back())；
3.修改队列的操作(push(),pop()和emplace())。也会遇到在固有接口上的条件竞争。
需要将front()和pop()合并成一个函数调用，就像之前在栈实现时合并top()和pop()一
样。
不同的是：当使用队列在多个线程中传递数据时，接收线程通常需要等待数据的压入。
这里提供pop()函数的两个变种：try_pop()和wait_and_pop()。
try_pop() ，尝试从队列中弹出数据，总会直接返回(当有失败时)，即使没有指可检索；
wait_and_pop()，将会等待有值可检索的时候才返回。
*/
#include<memory>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {

private:
	mutable std::mutex mut; //互斥量必须是可变的
	queue<T> data_queue;
	condition_variable data_cond;


public:
	threadsafe_queue() {}
	threadsafe_queue(const threadsafe_queue& other) {
		std::lock_guard<mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	threadsafe_queue& operator=(
		const threadsafe_queue&) = delete; // 不允许简单的赋值

	void push(T new_value) {
		std::lock_guard<mutex> lk(mut);
		data_queue.push(new_value);
		data_cond.notify_one();
	}
	bool try_pop(T& value) {
		lock_guard<mutex> lk(mut);
		if (data_queue.empty()) {
			return false;
		}
		value = data_queue.front();
		data_queue.pop();
		return true;
	} 

	std::shared_ptr<T> try_pop() {
		lock_guard<mutex> lk(mut);
		if (data_queue.empty()) {
			return shared_ptr<T>();
		}
		shared_ptr<T> ptr(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return ptr;
	} 

	void wait_and_pop(T& value) {
		unique_lock<mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		value = data_queue.front();
		data_queue.pop();
	}

	std::shared_ptr<T> wait_and_pop() {
		unique_lock<mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool empty() const {
		lock_guard<mutex> lk(mut);
		return data_queue.empty();
	}

};