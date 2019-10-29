#include<condition_variable>
#include<mutex>
#include<queue>
using namespace std;


//ʹ�� std::condition_variable �������ݵȴ�
class data_chunk {};
mutex mu;
queue<data_chunk> data_queue;
condition_variable data_cond;

void data_preparation_thread() {
	while (more_data_to_prepare()) {
		data_chunk const data = prepare_data();
		lock_guard<mutex> lk(mu);
		
		data_queue.push(data);
		data_cond.notify_one(); //�Եȴ����߳̽���֪ͨ������У�
	}
}

void data_processing_thread() {
	while (true) {
		/**
		Ϊʲôʹ��unique_lock?
		�ȴ��е��̱߳����ڵȴ��ڼ��������������������֮��Ի������ٴ��������� std::lock_guard û����ô��(lock_guardֻ��������ʱ�Ż����)
		*/
		unique_lock<mutex> lk(mu);
		data_cond.wait(lk, [] {return !data_queue.empty(); });//lambda��Ϊ�ȴ������� ��������lambda������Ὣ�߳�����������ȴ�״̬��wait�Ի��������н���
		data_chunk data = data_queue.front();
		data_queue.pop();
		lk.unlock(); //�����д�������δ���������
		process(data);
		if (is_last_chunk(data))
			break;
	}
}

//ʹ�� std::future ���첽�����л�ȡ����ֵ
#include<future>
#include<iostream>
int find_the_answer_to_ltuae();
void do_other_stuff();
void get_return_from_async() {
	future<int> the_answer = async(find_the_answer_to_ltuae);
	do_other_stuff();
	cout << "The answer is " << the_answer.get() << endl;
}

//ʹ�� std::async �������ݲ���
/**
std::async ������ͨ����Ӷ���ĵ��ò������������ݶ���Ĳ�����
����һ��������һ��ָ���Ա������ָ�룬�ڶ��������ṩ�����������Ա��ľ������(����ֱ�ӵģ�����ͨ��ָ�룬�����԰�װ�� std::ref ��)��ʣ��Ĳ�������Ϊ��Ա�����Ĳ������롣
���򣬵ڶ��������Ĳ�������Ϊ�����Ĳ���������Ϊָ���ɵ��ö���ĵ�һ��������
*/
#include<string>
struct X {
	void foo(int, string const&);
	string bar(string const&);
};

X x;
auto f1 = std::async(&X::foo, &x, 42, "hello"); // ����p->foo(42, "hello")��p��ָ��x��ָ��
auto f2 = std::async(&X::bar, x, "goodbye"); // ����tmpx.bar("goodbye")�� tmpx��x�Ŀ�������

struct Y
{
	double operator()(double);
};
Y y;

auto f3 = std::async(Y(), 3.141); // ����tmpy(3.141)��tmpyͨ��Y���ƶ����캯���õ�
auto f4 = std::async(std::ref(y), 2.718); // ����y(2.718)

X baz(X&);
auto f6 = std::async(baz, std::ref(x)); // ����baz(x)
class move_only
{
public:
	move_only();
	move_only(move_only&&);
	move_only(move_only const&) = delete;
	move_only& operator=(move_only&&);
	move_only& operator=(move_only const&) = delete;
	void operator()();
};
auto f5 = std::async(move_only());//����tmp()������tmp�Ǵ�std::move(move_only())�����

/**
��Ĭ������£���ȡ���� std::async �Ƿ�����һ���̣߳����Ƿ��������ȴ�ʱͬ��������
����������(�������������Ҫ�Ľ��)��������Ҳ�����ں�������֮ǰ���� std::async ��
��һ�������������������������� std::launch ���������� std::launch::defered ��������
���������ñ��ӳٵ�wait()��get()��������ʱ��ִ�У� std::launch::async ����������������
���ڵĶ����߳���ִ�У� std::launch::deferred | std::launch::async ����ʵ�ֿ���ѡ������
�ַ�ʽ��һ�֡����һ��ѡ����Ĭ�ϵġ����������ñ��ӳ٣������ܲ����������ˡ�
*/
auto f6 = std::async(std::launch::async, Y(), 1.2); // �����߳���ִ��
auto f7 = std::async(std::launch::deferred, baz, std::ref(x)); // ��wait()��get()����ʱִ��
auto f8 = std::async(std::launch::deferred | std::launch::async,baz, std::ref(x)); // ʵ��ѡ��ִ�з�ʽ
auto f9 = std::async(baz, std::ref(x));
f7.wait(); // �����ӳٺ���

//std::packaged_task<> ���ػ������ֲ��ඨ��
template<>
class packaged_task<std::string(std::vector<char>*, int)>
{
public:
	template<typename Callable>
	explicit packaged_task(Callable&& f);
	std::future<std::string> get_future();
	void operator()(std::vector<char>*, int);
};

//ʹ�� std::packaged_task ִ��һ��ͼ�ν����߳�
#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>
std::mutex m;
std::deque<std::packaged_task<void()> > tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread() // 1
{
	while (!gui_shutdown_message_received()) // 2
	{
		get_and_process_gui_message(); // 3
		std::packaged_task<void()> task;
		{
			std::lock_guard<std::mutex> lk(m);
			if (tasks.empty()) // 4
				continue;
			task = std::move(tasks.front()); // 5
			tasks.pop_front();
		}
		task(); // 6
	}
}
std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
	std::packaged_task<void()> task(f); // 7
	std::future<void> res = task.get_future(); // 8
	std::lock_guard<std::mutex> lk(m); // 9
	tasks.push_back(std::move(task)); // 10
	return res;
}