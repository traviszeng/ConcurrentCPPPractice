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


/**
future 是一个能从其他地方获取到一个值的对象，如果是在不同的线程中，则被synchronizing properly.

std::condition_variable 可以用于异步事件的重复通知，但是有些时候可能只等待事件发生一次，比如：等待特定的航班，用条件变量大杀器有点浪费了。
C++11 标准库提供了几种异步任务机制。通常 thread 不能返回线程执行的结果(可以通过引用参数返回)，而在异步处理当中很多时候都需要获得计算的结果。
如果只获取结果一次那么选用 future，即通过 future 获取了结果后，后续再通过此 future 获取结果将会出错。
*/
/**
std::future可用于异步任务中获取任务结果，但是它只是获取结果而已，真正的异步调用需要配合 std::async, std::promise, std::packaged_task。
这里 async 是个模板函数，promise 和 packaged_task 是模板类，通常模板实例化参数是任务函数(callable object)。
*/

//async + future基本用法
int task(int a) { return 10; }
//futrue 的使用形式：future<int> myFuture=async(task,10) 
//async 的效果是：自动创建一个后台线程(可以选取一个空闲的线程), 将来会在某一时刻执行任务 task 函数，并将计算结果保存在 myFuture 中
// future::operator=
#include <iostream>       // std::cout
#include <future>         // std::async, std::future

int get_value() { return 10; }

int main()
{
	std::future<int> fut;           // default-constructed, 默认构造的 future 对象是无效的

  // async()返回的就是一个 uture 对象。 fut = future_returned_by_async(), 调用 future 的赋值运算符。
  // move-assigned，赋值运算符隐式使用的是 move 语意(非c++98的拷贝语意)， fut = 这使得 fut 变的有效。同时使得右操作数变的无效
	fut = std::async(get_value);
	//async创建并运行一个线程，返回一个与函数返回值相对应类型的 future，通过它我们可以在其他任何地方获取异步结果

	//Calling future::get on a valid future blocks the thread until the provider makes the shared state ready return 0;
	std::cout << "value: " << fut.get() << '\n';
}

//packaged_task + future
/**
packaged_task 用来包裹一个可调用的对象(包括函数，函数指针，函数对象/仿函数，成员函数等)
*/
packaged_task<int(int)> myPackaged(task);
//首先创建packaged_task对象myPackaged，其内部创建一个函数task和一个共享状态(用于返回task的结果)  
future<int> myFuture = myPackaged.get_future();
//通过 packaged_task::get_future() 返回一个future对象myFuture用于获取task的任务结果  
thread myThread(move(myPackaged), "hello world");
//创建一个线程执行task任务，这里注意move语义强制将左值转为右值使用因为packaged_task禁止copy constructor，可以不创建线程，那么task任务的执行将和future结果的获取在同一个线程，这样就不叫异步了  
//这里主线程可以做其它的操作  
int x = myFuture.get();
//线程还可以在执行一些其它操作，直到其想获取task的结果时调用此语句

//promise + future
#include <iostream>       // std::cout  
#include <functional>     // std::ref  
#include <thread>         // std::thread  
#include <future>         // std::promise, std::future  

void print_int(std::future<int>& fut) {
	int x = fut.get();//当promise::set_value()设置了promise的共享状态值后，
	// fut将会通过future::get()获得该共享状态值，若promise没有设置该值那么
	// fut.get()将会阻塞线程直到共享状态值被promise设置  
	std::cout << "value: " << x << '\n';//输出：<span style="font-family: monospace; white-space: pre; rgb(231, 231, 231);">value: 10</span>  
}

int main()
{
	std::promise<int> prom; //创建一个promise对象  
	std::future<int> fut = prom.get_future(); //获取promise内部的future，fut将和promise共享promise中的共享状态，
	// 该共享状态用于返回计算结果  
	std::thread th1(print_int, std::ref(fut));  //创建一个线程，并通过引用方式将fut传到print_int中  
	prom.set_value(10);    //设置共享状态值  
												 //  
	th1.join();//等待子线程  
	return 0;
}
/**
将主线程即需要 task 结果的线程称为 provider，称执行任务 task 或上面 print_int 的线程为 executor (这里只是为了后面表述方便，没有科学考证的)。从上面的例子可以看出，简单的同步机制都是通过设置某种共享状态然后通过 future 获取该共享状态达到同步

　　async 通过创建或者选取一个当前空闲线程执行任务，然后将计算结果保存至与此 async 相关的 future 中，期间只有存取结果值，没有其它的交互，并且是 provider 持有future，executor 执行任务

　　packaged_task 是一个对象其内部持有 callable object，provider 创建一个下线程 executor 执行任务，最后 provider 通过相关的 future 获取任务计算结果。和 async 差不多。只有任务结果的存取，没有其它交互

　　promise 是 provider 持有，executor 持有相关的 future，然后 provider 通过 promise 设定共享状态的值，future 获取该共享值后执行某些任务。
*/


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
/**std::packaged_task<void()> 创建任务，其包含了一个无参数无返回值的函数或
可调用对象(如果当这个调用有返回值时，返回值会被丢弃)。*/
std::deque<std::packaged_task<void()> > tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread() // 1
{
	while (!gui_shutdown_message_received()) // 未收到关闭图形界面的消息就继续
	{
		get_and_process_gui_message(); // 轮训界面消息处理
		std::packaged_task<void()> task;
		{
			std::lock_guard<std::mutex> lk(m);
			if (tasks.empty()) // 若tasks中无任务将再次循环
				continue;
			task = std::move(tasks.front()); // 从队列中提取出一个任务
			tasks.pop_front();
		}
		task(); // 执行该任务
	}
	//期望和任务相关，当任务执行完成时，状态被置为就绪状态
}
std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f) //将一个任务传入队列
{
	std::packaged_task<void()> task(f); // 提供一个打包好的任务
	std::future<void> res = task.get_future(); // 调用get_future()获取期望对象
	std::lock_guard<std::mutex> lk(m); // 9
	tasks.push_back(std::move(task)); // 10
	return res;
}

//使用promise解决单线程多连接问题
#include<future>
void process_connections(connection_set& connections){
	while (!done(connections)) // 1
	{
		for (connection_iterator // 依次检查每个链接
			connection = connections.begin(), end = connections.end();
			connection != end;
			++connection)
		{
			if (connection->has_incoming_data()) // 是否有数据
			{
				data_packet data = connection->incoming();
				std::promise<payload_type>& p =
					connection->get_promise(data.id); // 一个ID映射到一个 std::promise (可能是在相关容器中进行的依次查找)
				p.set_value(data.payload);
			}
			if (connection->has_outgoing_data()) // 正在发送已入队的传出数据
			{
				outgoing_packet data =
					connection->top_of_outgoing_queue();
				connection->send(data.payload);
				data.promise.set_value(true); // 传输成功
			}
		}
	}
}

/**
shared_future
因为 std::future 是只移动的，所以其所有权可以
在不同的实例中互相传递，但是只有一个实例可以获得特定的同步结果；
而 std::shared_future 实例是可拷贝的，所以多个对象可以引用同一关联“期望”的结果。
*/
#include<cassert>

std::promise<int> p;
std::future<int> f(p.get_future());
assert(f.valid()); // 1 "期望" f 是合法的
std::shared_future<int> sf(std::move(f));
assert(!f.valid()); // 2 "期望" f 现在是不合法的
assert(sf.valid()); // 3 sf 现在是合法的
std::promise<std::string> p;
std::shared_future<std::string> sf(p.get_future()); // 1 隐式转移所有权

//std::future 有一个share()成员函数，
//可用来创建新的 std::shared_future ，并且可以直接转移“期望”的所有权。
#include<map>
std::promise< std::map< int, string>::iterator> p;
auto sf = p.get_future().share();

/**
	限定等待时间
*/
//等待一个条件变量——有超时功能
#include<chrono>
condition_variable cv;
bool done;
mutex m;

bool wait_loop() {
	auto const timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
	unique_lock<mutex> lk(m);

	while (!done) {
		if (cv.wait_until(lk, timeout) == cv_status::timeout) {
			break;
		}
	}
	return done;
	
}

/**
	使用同步操作简化代码
*/
//函数式编程版快排
//输入一个list 返回一个list
/**
将input列表小于divided_point的值移动到新列表lower_part④中。
其他数继续留在input列表中。而后，你可以使用递归调用⑤⑥的方式，对
两个列表进行排序
*/
#include<list>
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
	if (input.empty())
	{
		return input;
	}
	std::list<T> result;
	result.splice(result.begin(), input, input.begin()); // 将输入的首个元素(中间值)放入结果列表中
	T const& pivot = *result.begin(); // 2
	auto divide_point = std::partition(input.begin(), input.end(),
		[&](T const& t) {return t < pivot; }); // 3
	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(),
		divide_point); // 4
	auto new_lower(
		sequential_quick_sort(std::move(lower_part))); // 5
	auto new_higher(
		sequential_quick_sort(std::move(input))); // 6
	result.splice(result.end(), new_higher); // 7
	result.splice(result.begin(), new_lower); // 8
	return result;
}

//快排并行版
template<typename T>
std::list<T> parallel_quick_sort(list<T> input)
{
	if (input.empty()) {
		return input;
	}

	list<T> result;
	result.splice(result, begin(), input, input.begin()); //将第一个值作为标志
	T const& pivot = *result.begin();

	auto divide_point(std::partition(input.begin(), input.end(), [&](T const& t) {return t < pivot; }));

	list<T> lower_part;
	lower_part.splice(lower_part.end(), input.begin(), divide_point); //将比标志值小的部分拷贝到lower_part

	std::future<list<T>> new_lower( //当前线程不对小于“中间”值部分的列表进行排序，使用 std::async() 在另一线程对其进行排序。
		std::async(&parallel_quick_sort<T>, move(lower_part)));

	auto new_higher(parallel_quick_sort(move(input))); //使用递归排序

	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower.get()); // 4
	return result;
}


//spawn_task的简单实现
template<typename F, typename A>
std::future<std::result_of<F(A&&)>::type>
spawn_task(F&& f, A&& a)
{
	typedef std::result_of<F(A&&)>::type result_type;
	std::packaged_task<result_type(A&&)>
		task(std::move(f)));
		std::future<result_type> res(task.get_future());
		std::thread t(std::move(task), std::move(a));
		t.detach();
		return res;
}

//ATM逻辑类的简单实现
struct card_inserted
{
	std::string account;
};
class atm
{
	messaging::receiver incoming;
	messaging::sender bank;
	messaging::sender interface_hardware;
	void (atm::*state)();
	std::string account;
	std::string pin;
	void waiting_for_card() // 1
	{
		interface_hardware.send(display_enter_card()); // 2
		incoming.wait(). // 3
			handle<card_inserted>(
				[&](card_inserted const& msg) // 4
		{
			account = msg.account;
			pin = "";
			interface_hardware.send(display_enter_pin());
			state = &atm::getting_pin;
		}
		);
	}
	void getting_pin() {
		
		incoming.wait()
			.handle<digit_pressed>( // 1
				[&](digit_pressed const& msg)
		{
			unsigned const pin_length = 4;
			pin += msg.digit;
			if (pin.length() == pin_length)
			{
				bank.send(verify_pin(account, pin, incoming));
				state = &atm::verifying_pin;
			}
		}
				)
			.handle<clear_last_pressed>( // 2
				[&](clear_last_pressed const& msg)
		{
			if (!pin.empty())
			{
				pin.resize(pin.length() - 1);
			}
		}
				)
			.handle<cancel_pressed>( // 3
				[&](cancel_pressed const& msg)
		{
			state = &atm::done_processing;
		}
		);
		
	}
public:
	void run() // 5
	{
		state = &atm::waiting_for_card; // 6
		try
		{
			for (;;)
			{
				(this->*state)(); // 7
			}
		}
		catch (messaging::close_queue const&)
		{
		}
	}
};