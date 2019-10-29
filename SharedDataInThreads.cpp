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
	void do_something() {}
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
/**
 * �ײ�Ļ�ȡ��߲���޷�����
*/
hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);

int do_low_level_stuff() {
	return 0;
}

int low_level_func(){
	std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
	return do_low_level_stuff();
}

void high_level_stuff(int param) {}

void high_level_func(){
	std::lock_guard<hierarchical_mutex> lk(high_level_mutex);
	high_level_stuff(low_level_func());
}

void thread_a(){
	high_level_func();
}

hierarchical_mutex other_mutex(100);
void do_other_stuff() {}

f7.wait()
{
	return f7 .();
}

void other_stuff(){
	high_level_func();
	do_other_stuff();
}

void thread_b(){ //exception 
	std::lock_guard<hierarchical_mutex> lk(other_mutex);
	other_stuff();
}

//ʹ�� unique_lock��дswap
void swap(some_big_object &l, some_big_object & r) {}
class X_unique{
private:
	some_big_object detail;
	std::mutex m;
public:
	X_unique(some_big_object const& sd):detail(sd){}
	friend void swap(X_unique& l,X_unique& r){
		if(&l==&r){
			return ;
		}
		std::unique_lock<std::mutex> lock_a(l.m,std::defer_lock);
		std::unique_lock<std::mutex> lock_b(r.m,std::defer_lock);
		std::lock(lock_a,lock_b);
		swap(r.detail,l.detail);
	}
};

//std::unique_lock�ǿ��ƶ��ģ������ɸ�ֵ�����͡�
std::unique_lock<mutex> get_lock() {
	extern std::mutex some_mutex;
	std::unique_lock<mutex> lk(some_mutex);
	//prepare_data();
	return lk;
}

void process_data() {
	unique_lock<mutex> lk(get_lock());  //�������Զ������ƶ����캯��
	//do_something();
}

void get_and_process_data() {
	mutex mu;
	unique_lock<mutex> my_lock(mu);
	//some_class data_to_process = get_next_data_chunk();
	my_lock.unlock(); // 1 ��Ҫ����ס�Ļ�����Խ��process()�����ĵ���
	//result_type result = process(data_to_process);
	my_lock.lock(); // 2 Ϊ��д�����ݣ��Ի������ٴ�����
	//write_result(data_to_process, result);
}

//�Ƚϲ�������һ����סһ��������
class Y
{
private:
	int some_detail;
	mutable std::mutex m;
	int get_detail() const
	{
		std::lock_guard<std::mutex> lock_a(m); // 1
		return some_detail;
	}
public:
	Y(int sd) :some_detail(sd) {}
	friend bool operator==(Y const& lhs, Y const& rhs)
	{
		if (&lhs == &rhs)
			return true;
		int const lhs_value = lhs.get_detail(); // 2
		int const rhs_value = rhs.get_detail(); // 3
		return lhs_value == rhs_value; // 4
	}
};

//������2 3�� ����ֵ�ᱻ�޸ģ���ʱֻ�Ǳ�֤==Ϊĳһʱ�̵�״̬

/**
�����������ݵĳ�ʼ������
*/

//�ӳٳ�ʼ����ÿһ����������Ҫ�ȶ�Դ���м�飬
//Ϊ���˽������Ƿ񱻳�ʼ����Ȼ������ʹ��ǰ�����������Ƿ���Ҫ��ʼ��
//���̰߳汾
shared_ptr<some_data> resource_ptr;
void lazyInit() {
	if (!resource_ptr) {
		resource_ptr.reset(new some_data);//תΪ���߳�ʱ��Ҫ������������Ϊÿ���̱߳���ȴ���������Ϊ��ȷ������Դ�Ѿ���ʼ���ˡ�
	}
	resource_ptr->do_something;
}

//ʹ��һ�����������ӳٳ�ʼ��(�̰߳�ȫ)����
std::mutex resource_mutex;
void lazyInit()
{
	std::unique_lock<std::mutex> lk(resource_mutex); // �����߳��ڴ����л�
	if (!resource_ptr)
	{
		resource_ptr.reset(new some_data); // ֻ�г�ʼ��������Ҫ����
	}
	lk.unlock();
	resource_ptr->do_something();
}

//˫�ؼ����ģʽ
/**
�ⲿ�Ķ�ȡ����û�����ڲ���
д��������ͬ���ۡ���˾ͻ������������
*/
void undefined_behaviour_with_double_checked_locking()
{
	if (!resource_ptr) // 1
	{
		std::lock_guard<std::mutex> lk(resource_mutex);
		if (!resource_ptr) // 2
		{
			resource_ptr.reset(new some_data); // 3
		}
	}
	resource_ptr->do_something(); // 4
}


/*
ʹ��std::call_once��std::once_flag������
������ס������������ʽ�ļ��ָ�룬ÿ���߳�ֻ��Ҫʹ�� std::call_once 
�� std::call_once �Ľ���ʱ�����ܰ�ȫ��֪��ָ���Ѿ����������̳߳�ʼ���ˡ�
*/
std::once_flag resource_flag;
void init_resource() {
	resource_ptr.reset(new some_data);
}

void init_with_call_once() {
	std::call_once(resource_flag, init_resource); //��������һ�γ�ʼ��
	resource_ptr->do_something();
}

// ʹ�� std::call_once ��Ϊ���Ա���ӳٳ�ʼ��(�̰߳�ȫ)
/**
��һ������send_data()�ٻ�receive_data()�۵��߳���ɳ�ʼ�����̡�
ʹ�ó�Ա����open_connection()ȥ��ʼ������
*/
class X
{
private:
	connection_info connection_details;
	connection_handle connection;
	std::once_flag connection_init_flag;
	void open_connection()
	{
		connection = connection_manager.open(connection_details);
	}
public:
	X(connection_info const& connection_details_) :
		connection_details(connection_details_)
	{}
	void send_data(data_packet const& data) // 1
	{
		std::call_once(connection_init_flag, &X::open_connection, this); // 2
		connection.send_data(data);
	}
	data_packet receive_data() // 3
	{
		std::call_once(connection_init_flag, &X::open_connection, this); // 2
		return connection.receive_data();
	}
};
/**
�� boost::shared_lock<boost::shared_mutex> ��ȡ�������Ȩ������ʹ�� std::unique_lock һ
�������Ƕ���Ҫ��ͬʱ�õ�ͬһ�� boost::shared_mutex ���й�������Ψһ�����ƾ��ǣ�����
һ�߳�ӵ��һ��������ʱ������߳̾ͻ᳢�Ի�ȡһ����ռ����ֱ�������̷߳������ǵ�
����ͬ���ģ�����һ�߳�ӵ��һ����ռ���ǣ������߳̾��޷���ù��������ռ����ֱ����
һ���̷߳�����ӵ�е�����
*/
// ʹ�� boost::shared_mutex �����ݽṹ���б���
#include <map>
#include <string>
#include <mutex>

#include <boost/thread.hpp>    
#include <boost/thread/mutex.hpp>

class dns_entry {};

class dns_cache {
	std::map<string, dns_entry> entries;
	mutable boost::shared_mutex entry_mutex;
public:
	/**ʹ���� boost::shared_lock<> ʵ���������乲���ֻ��Ȩ�ޢ٣�
	���ʹ�ã����߳̿���ͬʱ����find_entry()���Ҳ������*/
	dns_entry find_entry(string const& domain) const {
		boost::shared_lock<boost::shared_mutex> lk(entry_mutex); //1
		std::map<string, dns_entry>::const_iterator  const it = entries.find(domain);
		return (it == entries.end()) ? dns_entry() : it->second;
	}
	/**�������Ҫ����ʱ�ڣ�Ϊ���ṩ��ռ����Ȩ�ޣ�
	��update_or_add_entry()��������ʱ����ռ������ֹ�����̶߳����ݽṹ�����޸ģ�������Щ�߳�����ʱ��Ҳ���ܵ���find_entry()��*/
	void update_or_add_entry(std::string const& domain,dns_entry const& dns_details)
	{
		std::lock_guard<boost::shared_mutex> lk(entry_mutex); // 2
		entries[domain] = dns_details;
	}

};