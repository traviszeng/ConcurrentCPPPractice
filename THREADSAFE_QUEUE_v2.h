#pragma once
/**
持有 std::shared_ptr<> 实例的线程安全队列
*/
/**
std::shared_ptr<> 持有数据的好处：新的实例分配结束时，不会被锁在push()⑤当中(而在THREADSAFE_QUEUE中，只能在pop()持有锁时完成)。
因为内存分配操作的需要在性能上付出很高的代价(性能较低)，所以使用 std::shared_ptr<> 的方式对队列的性能有很大的提升，其减少了互斥量持有的时间，允许其他线程在分配内存的同时，对队列进行其他的操作。
*/
#include<memory>
#include <queue>
#include <mutex>
#include <condition_variable>
template<typename T>
class threadsafe_queue
{
private:
	mutable std::mutex mut;
	std::queue<std::shared_ptr<T> > data_queue;
	std::condition_variable data_cond;
public:
	threadsafe_queue()
	{}

	void wait_and_pop(T& value) {
		unique_lock<mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty()});
		value = std::move(*data_queue.front()); //返回一个引用
		data_queue.pop();
	}

	bool try_pop(T& val) {
		lock_guard<mutex> lk(mut);
		if (data_queue.empty()) { return false; }
		val = std::move(*data_queue.front()); //返回一个引用
		data_queue.pop();
		return true;
	}

	shared_ptr<T> wait_and_pop() {
		unique_lock<mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		shared_ptr<T> res = data_queue.front(); // 返回一个shared_ptr实例
		data_queue.pop();
		return res;
	}


	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res = data_queue.front(); // 返回一个shared_ptr实例
		data_queue.pop();
		return res;
	}

	void push(T new_val) {
		shared_ptr<T> data(std::make_shared<T>(move(new_val))); //5
		lock_guard<mutex> lk(mut);
		data_queue.push(data);
		data_cond.notify_one();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

