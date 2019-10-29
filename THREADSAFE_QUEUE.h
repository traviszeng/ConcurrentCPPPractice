#pragma once
/**
1. ���������е�״̬���в�ѯ(empty()��size());
2.��ѯ�ڶ����еĸ���Ԫ��(front()��back())��
3.�޸Ķ��еĲ���(push(),pop()��emplace())��Ҳ�������ڹ��нӿ��ϵ�����������
��Ҫ��front()��pop()�ϲ���һ���������ã�����֮ǰ��ջʵ��ʱ�ϲ�top()��pop()һ
����
��ͬ���ǣ���ʹ�ö����ڶ���߳��д�������ʱ�������߳�ͨ����Ҫ�ȴ����ݵ�ѹ�롣
�����ṩpop()�������������֣�try_pop()��wait_and_pop()��
try_pop() �����ԴӶ����е������ݣ��ܻ�ֱ�ӷ���(����ʧ��ʱ)����ʹû��ָ�ɼ�����
wait_and_pop()������ȴ���ֵ�ɼ�����ʱ��ŷ��ء�
*/
#include<memory>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {

private:
	mutable std::mutex mut; //�����������ǿɱ��
	queue<T> data_queue;
	condition_variable data_cond;


public:
	threadsafe_queue() {}
	threadsafe_queue(const threadsafe_queue& other) {
		std::lock_guard<mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	threadsafe_queue& operator=(
		const threadsafe_queue&) = delete; // ������򵥵ĸ�ֵ

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