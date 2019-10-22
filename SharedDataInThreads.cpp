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