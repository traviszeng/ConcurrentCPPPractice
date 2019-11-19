#pragma once
#include <exception>
#include <memory> // For std::shared_ptr<>
#include <mutex>
#include <stack>
struct empty_stack : std::exception
{
	const char* what() const throw() {
		return "empty stack!";
	};
};

/**线程安全的栈的实现*/
/*
 定期等待和检查empty()和pop()以及对于empty_stack异常进行关注会浪费栈的资源
 考虑使用条件变量来异步通知
 */
template<typename T>
class threadsafe_stack {
private:
	std::stack<T> data;
	mutable std::mutex m;
public:
	threadsafe_stack() :data(std::stack<int>()) {
		
	}
	threadsafe_stack(const threadsafe_stack& other) {
		std::lock_guard<std::mutex> guard(m);
		data = other.data; //执行拷贝
	}
	threadsafe_stack& operator=(const threadsafe_stack&) = delete;//复制操作被删除

	void push(T new_value) {
		std::lock_guard<std::mutex> guard(m);
		data.push(std::move(new_value)); //std::stack保证data.push的调用安全
	}
	//返回一个指向弹出元素的指针，而不是直接返回值
	//栈为空的时候pop函数抛出一个empty_stack异常
	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) throw empty_stack();//调用pop前，先检查栈是否为空

		std::shared_ptr<T> const res(std::make_shared<T>(data.top())); //修改堆栈前，分配出返回值
		data.pop();
		return res;
	}
	//使用一个局部引用去存储弹出值
	void pop(T& value) {
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

