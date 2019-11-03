#pragma once
using namespace std;
//用单链表实现的单线程版本的queue
template<typename T>
class queue{
private:
	struct node{
		T data;
		std::unique_ptr<node> next;
		node(T data_):data(std::move(data_)){}
	};

	std::unique<node> head;//1
	node* tail;	//2
public:
	queue(){}
	queue& operator=(const queue& other)=delete;
	queue(const queue& other) = delete;

	std::shared_ptr<T> try_pop(){
		if(!head){
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> const res(std::make_shared<T>(std::move(head->data)));

		std::unique_ptr<T> const old_head = std::move(head);
		head = std::move(old_head->next);//3
		return res;
	}

	void push(T new_val){
		std::unique_ptr<node> p(new node(std::move(new_val)));
		node* const new_tail = p.get();

		if(tail){
			tail->next = std::move(p); //4
		}
		else
		{
			head = std::move(p); //5
		}
		tail = new_tail;
		
	}


};


template<typename T>
class threadsafe_queue
{
private:
	struct node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};
	std::mutex head_mutex;
	std::unique_ptr<node> head;
	std::mutex tail_mutex;
	node* tail;
	node* get_tail()
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		return tail;
	}
	std::unique_ptr<node> pop_head()
	{
		std::lock_guard<std::mutex> head_lock(head_mutex);
		if (head.get() == get_tail())
		{
		return nullptr;
		}
		std::unique_ptr<node> old_head = std::move(head);
		head = std::move(old_head->next);
		return old_head;
	}
public:
	threadsafe_queue() :
		head(new node), tail(head.get())
	{}
	threadsafe_queue(const threadsafe_queue& other) = delete;
	threadsafe_queue& operator=(const threadsafe_queue& other) = delete;
	std::shared_ptr<T> try_pop()
	{
		std::unique_ptr<node> old_head = pop_head();
		return old_head ? old_head->data : std::shared_ptr<T>();
	}
	void push(T new_value)
	{
		std::shared_ptr<T> new_data(
			std::make_shared<T>(std::move(new_value)));
		std::unique_ptr<node> p(new node);
		node* const new_tail = p.get();
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		tail->data = new_data;
		tail->next = std::move(p);
		tail = new_tail;
	}
};