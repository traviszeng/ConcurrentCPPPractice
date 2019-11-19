#pragma once
/**使用条件变量实现的线程安全队列*/
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
	std::condition_variable data_cond;


public:
	threadsafe_queue() {}
	threadsafe_queue(const threadsafe_queue& other) {
		std::lock_guard<std::mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	threadsafe_queue& operator=(
		const threadsafe_queue&) = delete; // 不允许简单的赋值

	void push(T new_value) {
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(new_value);
		data_cond.notify_one();
	}
	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) {
			return false;
		}
		value = std::move(data_queue.front());
		data_queue.pop();
		return true;
	} 

	std::shared_ptr<T> try_pop() {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> res(
			std::make_shared<T>(std::move(data_queue.front())));
		data_queue.pop();
		return res;
	} 

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		value = std::move(data_queue.front());
		data_queue.pop();
	}
	/**
		若得到notify_one的线程在wait_and_pop中抛出异常，则其他线程将继续休眠，无法再次得到notify
		方案一：
			当这种情况是不可接受时，这里的调用就需要改成
			data_cond.notify_all()，这个函数将唤醒所有的工作线程，不过，当大多线程发现队列依旧是
			空时，又会耗费很多资源让线程重新进入睡眠状态。
		方案二：
			当有异常抛出的时候，让wait_and_pop()函数调用notify_one()，从而让个另一个线程可以去尝试索引存储的值。
		方案三：
			将 std::shared_ptr<> 的初始化过程移到push()中，并且存储 std::shared_ptr<> 实例，而非直接使用数据的值。
			将 std::shared_ptr<> 拷贝到内部 std::queue<> 中，就不会抛出异常了，这样wait_and_pop()又是安全的了。
		使用方案三进行改进见THREADSAFE_QUEUE_shared_ptr.h
	*/
	std::shared_ptr<T> wait_and_pop() { 
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		std::shared_ptr<T> res(
			std::make_shared<T>(std::move(data_queue.front())));
		data_queue.pop();
		return res;
	}
	bool empty() const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}

};
