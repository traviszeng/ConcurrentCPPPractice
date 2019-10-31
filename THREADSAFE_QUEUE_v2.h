#pragma once
/**
���� std::shared_ptr<> ʵ�����̰߳�ȫ����
*/
/**
std::shared_ptr<> �������ݵĺô����µ�ʵ���������ʱ�����ᱻ����push()�ݵ���(����THREADSAFE_QUEUE�У�ֻ����pop()������ʱ���)��
��Ϊ�ڴ�����������Ҫ�������ϸ����ܸߵĴ���(���ܽϵ�)������ʹ�� std::shared_ptr<> �ķ�ʽ�Զ��е������кܴ��������������˻��������е�ʱ�䣬���������߳��ڷ����ڴ��ͬʱ���Զ��н��������Ĳ�����
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
		value = std::move(*data_queue.front()); //����һ������
		data_queue.pop();
	}

	bool try_pop(T& val) {
		lock_guard<mutex> lk(mut);
		if (data_queue.empty()) { return false; }
		val = std::move(*data_queue.front()); //����һ������
		data_queue.pop();
		return true;
	}

	shared_ptr<T> wait_and_pop() {
		unique_lock<mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		shared_ptr<T> res = data_queue.front(); // ����һ��shared_ptrʵ��
		data_queue.pop();
		return res;
	}


	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res = data_queue.front(); // ����һ��shared_ptrʵ��
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

