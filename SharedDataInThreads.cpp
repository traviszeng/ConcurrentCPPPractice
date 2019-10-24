#include<iostream>
using namespace std;
/**
ʹ��lock_guard�û����������л������
*/

/*ʹ�û����������б�*/
/**
�������������к���Ҫ����������ͬʱ����Ϊprivate
�����г�Ա���������ڵ���ʱ����������������ʱ�����ݽ�������ô�ͱ�֤�����ݷ���ʱ�����������ƻ���

���������⣺
������һ����Ա�������ص��Ǳ�����
�ݵ�ָ�������ʱ�����ƻ������ݵı��������з���������ָ������ÿ��Է���(�������޸�)
�����������ݣ������ᱻ���������ơ�������������������Ҫ�Խӿڵ�����൱������Ҫȷ
������������ס�κζԱ������ݵķ��ʣ����Ҳ������š�
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
������������Ϊһ������ʱ����,�����д����˱������ݵ�����
*/
/**
�����ܱ������ݵ�ָ������ô��ݵ�������������֮�⣬����
�Ǻ�������ֵ�����Ǵ洢���ⲿ�ɼ��ڴ棬������Բ�������ʽ���ݵ��û��ṩ�ĺ�����
ȥ��
*/

class some_data
{
	int a;
	std::string b;
public:
	void do_something();
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
		func(data); // 1 ���ݡ����������ݸ��û�����
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
	x.process_data(malicious_function); // 2 ����һ�����⺯��
	unprotected->do_something(); // 3 ���ޱ���������·��ʱ�������
}

/**�̰߳�ȫ��ջ*/
#include "threadsafe_stack.h"

/**ʹ��std::lock�������*/
// �����std::lock()��Ҫ����<mutex>ͷ�ļ�
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
		std::lock(lhs.m, rhs.m); // ��std::lock��ס����������������������Ҫôȫ����ס��Ҫôһ��������
		std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock); // �ṩ std::adopt_lock �������˱�ʾ std::lock_guard �����Ѿ������⣬����ʾ�ֳɵ��������ǳ��Դ����µ�����
		std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock); 
		swap(lhs.some_detail, rhs.some_detail);
	}
};

/**ʹ�ò��������������*/
#include"HIERARCHICAL_MUTEX.h"
hierarchical_mutex high_level_mutex(10000); // 1
hierarchical_mutex low_level_mutex(5000); // 2
int do_low_level_stuff();
int low_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex); // 3
	return do_low_level_stuff();
}
void high_level_stuff(int some_param);
void high_level_func()
{
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex); // 4
	high_level_stuff(low_level_func()); // 5
}
void thread_a() // 6
{
	high_level_func();
}
hierarchical_mutex other_mutex(100); // 7
void do_other_stuff();
void other_stuff()
{
	high_level_func(); // 8
	do_other_stuff();
}
void thread_b() // 9
{
	std::lock_guard<hierarchical_mutex> lk(other_mutex); // 10
	other_stuff();
}