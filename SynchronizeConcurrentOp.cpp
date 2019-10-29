#include<condition_variable>
#include<mutex>
#include<queue>
using namespace std;


//使用 std::condition_variable 处理数据等待
class data_chunk {};
mutex mu;
queue<data_chunk> data_queue;
condition_variable data_cond;

void data_preparation_thread() {
	while (more_data_to_prepare()) {
		data_chunk const data = prepare_data();
		lock_guard<mutex> lk(mu);
		
		data_queue.push(data);
		data_cond.notify_one(); //对等待的线程进行通知（如果有）
	}
}

void data_processing_thread() {
	while (true) {
		/**
		为什么使用unique_lock?
		等待中的线程必须在等待期间解锁互斥量，并在这这之后对互斥量再次上锁，而 std::lock_guard 没有这么灵活。(lock_guard只有在析构时才会解锁)
		*/
		unique_lock<mutex> lk(mu);
		data_cond.wait(lk, [] {return !data_queue.empty(); });//lambda作为等待的条件 若不满足lambda条件则会将线程置于阻塞或等待状态，wait对互斥锁进行解锁
		data_chunk data = data_queue.front();
		data_queue.pop();
		lk.unlock(); //用于有待处理但还未处理的数据
		process(data);
		if (is_last_chunk(data))
			break;
	}
}

//使用 std::future 从异步任务中获取返回值
#include<future>
#include<iostream>
int find_the_answer_to_ltuae();
void do_other_stuff();
void get_return_from_async() {
	future<int> the_answer = async(find_the_answer_to_ltuae);
	do_other_stuff();
	cout << "The answer is " << the_answer.get() << endl;
}

//使用 std::async 向函数传递参数
/**
std::async 允许你通过添加额外的调用参数，向函数传递额外的参数。
当第一个参数是一个指向成员函数的指针，第二个参数提供有这个函数成员类的具体对象(不是直接的，就是通过指针，还可以包装在 std::ref 中)，剩余的参数可作为成员函数的参数传入。
否则，第二个和随后的参数将作为函数的参数，或作为指定可调用对象的第一个参数。
*/
#include<string>
struct X {
	void foo(int, string const&);
	string bar(string const&);
};

X x;
auto f1 = std::async(&X::foo, &x, 42, "hello"); // 调用p->foo(42, "hello")，p是指向x的指针
auto f2 = std::async(&X::bar, x, "goodbye"); // 调用tmpx.bar("goodbye")， tmpx是x的拷贝副本

struct Y
{
	double operator()(double);
};
Y y;

auto f3 = std::async(Y(), 3.141); // 调用tmpy(3.141)，tmpy通过Y的移动构造函数得到
auto f4 = std::async(std::ref(y), 2.718); // 调用y(2.718)

X baz(X&);
auto f6 = std::async(baz, std::ref(x)); // 调用baz(x)
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
auto f5 = std::async(move_only());//调用tmp()，其中tmp是从std::move(move_only())构造的

/**
在默认情况下，这取决于 std::async 是否启动一个线程，或是否在期望等待时同步任务。在
大多数情况下(估计这就是你想要的结果)，但是你也可以在函数调用之前，向 std::async 传
递一个额外参数。这个参数的类型是 std::launch ，还可以是 std::launch::defered ，用来表
明函数调用被延迟到wait()或get()函数调用时才执行， std::launch::async 表明函数必须在其
所在的独立线程上执行， std::launch::deferred | std::launch::async 表明实现可以选择这两
种方式的一种。最后一个选项是默认的。当函数调用被延迟，它可能不会在运行了。
*/
auto f6 = std::async(std::launch::async, Y(), 1.2); // 在新线程上执行
auto f7 = std::async(std::launch::deferred, baz, std::ref(x)); // 在wait()或get()调用时执行
auto f8 = std::async(std::launch::deferred | std::launch::async,baz, std::ref(x)); // 实现选择执行方式
auto f9 = std::async(baz, std::ref(x));
f7.wait(); // 调用延迟函数

//std::packaged_task<> 的特化——局部类定义
template<>
class packaged_task<std::string(std::vector<char>*, int)>
{
public:
	template<typename Callable>
	explicit packaged_task(Callable&& f);
	std::future<std::string> get_future();
	void operator()(std::vector<char>*, int);
};

//使用 std::packaged_task 执行一个图形界面线程
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