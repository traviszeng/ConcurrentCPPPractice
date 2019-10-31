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

/**�̰߳�ȫ��ջ��ʵ��*/
template<typename T>
class threadsafe_stack {
private:
	std::stack<T> data;
	mutable std::mutex m;
public:
	threadsafe_stack() :data(std::stack<int>) {
		
	}
	threadsafe_stack(const threadsafe_stack& other) {
		std::lock_guard<std::mutex> guard;
		data = other.data; //ִ�п���
	}
	threadsafe_stack& operator=(const threadsafe_stack&) = delete;//���Ʋ�����ɾ��

	void push(T new_value) {
		std::lock_guard<std::mutex> guard;
		data.push(std::move(new_value)); //std::stack��֤data.push�ĵ��ð�ȫ
	}
	//����һ��ָ�򵯳�Ԫ�ص�ָ�룬������ֱ�ӷ���ֵ
	//ջΪ�յ�ʱ��pop�����׳�һ��empty_stack�쳣
	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> lock(m);
		if (data.empty()) throw empty_stack();//����popǰ���ȼ��ջ�Ƿ�Ϊ��

		std::shared_ptr<T> const res(std::make_shared<T>(data.top())); //�޸Ķ�ջǰ�����������ֵ
		data.pop();
		return res;
	}
	//ʹ��һ���ֲ�����ȥ�洢����ֵ
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

