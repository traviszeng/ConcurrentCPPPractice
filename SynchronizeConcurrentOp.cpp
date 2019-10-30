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


/**
future ��һ���ܴ������ط���ȡ��һ��ֵ�Ķ���������ڲ�ͬ���߳��У���synchronizing properly.

std::condition_variable ���������첽�¼����ظ�֪ͨ��������Щʱ�����ֻ�ȴ��¼�����һ�Σ����磺�ȴ��ض��ĺ��࣬������������ɱ���е��˷��ˡ�
C++11 ��׼���ṩ�˼����첽������ơ�ͨ�� thread ���ܷ����߳�ִ�еĽ��(����ͨ�����ò�������)�������첽�����кܶ�ʱ����Ҫ��ü���Ľ����
���ֻ��ȡ���һ����ôѡ�� future����ͨ�� future ��ȡ�˽���󣬺�����ͨ���� future ��ȡ����������
*/
/**
std::future�������첽�����л�ȡ��������������ֻ�ǻ�ȡ������ѣ��������첽������Ҫ��� std::async, std::promise, std::packaged_task��
���� async �Ǹ�ģ�庯����promise �� packaged_task ��ģ���࣬ͨ��ģ��ʵ����������������(callable object)��
*/

//async + future�����÷�
int task(int a) { return 10; }
//futrue ��ʹ����ʽ��future<int> myFuture=async(task,10) 
//async ��Ч���ǣ��Զ�����һ����̨�߳�(����ѡȡһ�����е��߳�), ��������ĳһʱ��ִ������ task ���������������������� myFuture ��
// future::operator=
#include <iostream>       // std::cout
#include <future>         // std::async, std::future

int get_value() { return 10; }

int main()
{
	std::future<int> fut;           // default-constructed, Ĭ�Ϲ���� future ��������Ч��

  // async()���صľ���һ�� uture ���� fut = future_returned_by_async(), ���� future �ĸ�ֵ�������
  // move-assigned����ֵ�������ʽʹ�õ��� move ����(��c++98�Ŀ�������)�� fut = ��ʹ�� fut �����Ч��ͬʱʹ���Ҳ����������Ч
	fut = std::async(get_value);
	//async����������һ���̣߳�����һ���뺯������ֵ���Ӧ���͵� future��ͨ�������ǿ����������κεط���ȡ�첽���

	//Calling future::get on a valid future blocks the thread until the provider makes the shared state ready return 0;
	std::cout << "value: " << fut.get() << '\n';
}

//packaged_task + future
/**
packaged_task ��������һ���ɵ��õĶ���(��������������ָ�룬��������/�º�������Ա������)
*/
packaged_task<int(int)> myPackaged(task);
//���ȴ���packaged_task����myPackaged�����ڲ�����һ������task��һ������״̬(���ڷ���task�Ľ��)  
future<int> myFuture = myPackaged.get_future();
//ͨ�� packaged_task::get_future() ����һ��future����myFuture���ڻ�ȡtask��������  
thread myThread(move(myPackaged), "hello world");
//����һ���߳�ִ��task��������ע��move����ǿ�ƽ���ֵתΪ��ֵʹ����Ϊpackaged_task��ֹcopy constructor�����Բ������̣߳���ôtask�����ִ�н���future����Ļ�ȡ��ͬһ���̣߳������Ͳ����첽��  
//�������߳̿����������Ĳ���  
int x = myFuture.get();
//�̻߳�������ִ��һЩ����������ֱ�������ȡtask�Ľ��ʱ���ô����

//promise + future
#include <iostream>       // std::cout  
#include <functional>     // std::ref  
#include <thread>         // std::thread  
#include <future>         // std::promise, std::future  

void print_int(std::future<int>& fut) {
	int x = fut.get();//��promise::set_value()������promise�Ĺ���״ֵ̬��
	// fut����ͨ��future::get()��øù���״ֵ̬����promiseû�����ø�ֵ��ô
	// fut.get()���������߳�ֱ������״ֵ̬��promise����  
	std::cout << "value: " << x << '\n';//�����<span style="font-family: monospace; white-space: pre; rgb(231, 231, 231);">value: 10</span>  
}

int main()
{
	std::promise<int> prom; //����һ��promise����  
	std::future<int> fut = prom.get_future(); //��ȡpromise�ڲ���future��fut����promise����promise�еĹ���״̬��
	// �ù���״̬���ڷ��ؼ�����  
	std::thread th1(print_int, std::ref(fut));  //����һ���̣߳���ͨ�����÷�ʽ��fut����print_int��  
	prom.set_value(10);    //���ù���״ֵ̬  
												 //  
	th1.join();//�ȴ����߳�  
	return 0;
}
/**
�����̼߳���Ҫ task ������̳߳�Ϊ provider����ִ������ task ������ print_int ���߳�Ϊ executor (����ֻ��Ϊ�˺���������㣬û�п�ѧ��֤��)������������ӿ��Կ������򵥵�ͬ�����ƶ���ͨ������ĳ�ֹ���״̬Ȼ��ͨ�� future ��ȡ�ù���״̬�ﵽͬ��

����async ͨ����������ѡȡһ����ǰ�����߳�ִ������Ȼ�󽫼�������������� async ��ص� future �У��ڼ�ֻ�д�ȡ���ֵ��û�������Ľ����������� provider ����future��executor ִ������

����packaged_task ��һ���������ڲ����� callable object��provider ����һ�����߳� executor ִ��������� provider ͨ����ص� future ��ȡ������������� async ��ࡣֻ���������Ĵ�ȡ��û����������

����promise �� provider ���У�executor ������ص� future��Ȼ�� provider ͨ�� promise �趨����״̬��ֵ��future ��ȡ�ù���ֵ��ִ��ĳЩ����
*/


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
/**std::packaged_task<void()> ���������������һ���޲����޷���ֵ�ĺ�����
�ɵ��ö���(�������������з���ֵʱ������ֵ�ᱻ����)��*/
std::deque<std::packaged_task<void()> > tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();
void gui_thread() // 1
{
	while (!gui_shutdown_message_received()) // δ�յ��ر�ͼ�ν������Ϣ�ͼ���
	{
		get_and_process_gui_message(); // ��ѵ������Ϣ����
		std::packaged_task<void()> task;
		{
			std::lock_guard<std::mutex> lk(m);
			if (tasks.empty()) // ��tasks���������ٴ�ѭ��
				continue;
			task = std::move(tasks.front()); // �Ӷ�������ȡ��һ������
			tasks.pop_front();
		}
		task(); // ִ�и�����
	}
	//������������أ�������ִ�����ʱ��״̬����Ϊ����״̬
}
std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f) //��һ�����������
{
	std::packaged_task<void()> task(f); // �ṩһ������õ�����
	std::future<void> res = task.get_future(); // ����get_future()��ȡ��������
	std::lock_guard<std::mutex> lk(m); // 9
	tasks.push_back(std::move(task)); // 10
	return res;
}

//ʹ��promise������̶߳���������
#include<future>
void process_connections(connection_set& connections){
	while (!done(connections)) // 1
	{
		for (connection_iterator // ���μ��ÿ������
			connection = connections.begin(), end = connections.end();
			connection != end;
			++connection)
		{
			if (connection->has_incoming_data()) // �Ƿ�������
			{
				data_packet data = connection->incoming();
				std::promise<payload_type>& p =
					connection->get_promise(data.id); // һ��IDӳ�䵽һ�� std::promise (����������������н��е����β���)
				p.set_value(data.payload);
			}
			if (connection->has_outgoing_data()) // ���ڷ�������ӵĴ�������
			{
				outgoing_packet data =
					connection->top_of_outgoing_queue();
				connection->send(data.payload);
				data.promise.set_value(true); // ����ɹ�
			}
		}
	}
}

/**
shared_future
��Ϊ std::future ��ֻ�ƶ��ģ�����������Ȩ����
�ڲ�ͬ��ʵ���л��ഫ�ݣ�����ֻ��һ��ʵ�����Ի���ض���ͬ�������
�� std::shared_future ʵ���ǿɿ����ģ����Զ�������������ͬһ�������������Ľ����
*/
#include<cassert>

std::promise<int> p;
std::future<int> f(p.get_future());
assert(f.valid()); // 1 "����" f �ǺϷ���
std::shared_future<int> sf(std::move(f));
assert(!f.valid()); // 2 "����" f �����ǲ��Ϸ���
assert(sf.valid()); // 3 sf �����ǺϷ���
std::promise<std::string> p;
std::shared_future<std::string> sf(p.get_future()); // 1 ��ʽת������Ȩ

//std::future ��һ��share()��Ա������
//�����������µ� std::shared_future �����ҿ���ֱ��ת�ơ�������������Ȩ��
#include<map>
std::promise< std::map< int, string>::iterator> p;
auto sf = p.get_future().share();

/**
	�޶��ȴ�ʱ��
*/
//�ȴ�һ���������������г�ʱ����
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
	ʹ��ͬ�������򻯴���
*/
//����ʽ��̰����
//����һ��list ����һ��list
/**
��input�б�С��divided_point��ֵ�ƶ������б�lower_part���С�
��������������input�б��С����������ʹ�õݹ���âݢ޵ķ�ʽ����
�����б��������
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
	result.splice(result.begin(), input, input.begin()); // ��������׸�Ԫ��(�м�ֵ)�������б���
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

//���Ų��а�
template<typename T>
std::list<T> parallel_quick_sort(list<T> input)
{
	if (input.empty()) {
		return input;
	}

	list<T> result;
	result.splice(result, begin(), input, input.begin()); //����һ��ֵ��Ϊ��־
	T const& pivot = *result.begin();

	auto divide_point(std::partition(input.begin(), input.end(), [&](T const& t) {return t < pivot; }));

	list<T> lower_part;
	lower_part.splice(lower_part.end(), input.begin(), divide_point); //���ȱ�־ֵС�Ĳ��ֿ�����lower_part

	std::future<list<T>> new_lower( //��ǰ�̲߳���С�ڡ��м䡱ֵ���ֵ��б��������ʹ�� std::async() ����һ�̶߳����������
		std::async(&parallel_quick_sort<T>, move(lower_part)));

	auto new_higher(parallel_quick_sort(move(input))); //ʹ�õݹ�����

	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower.get()); // 4
	return result;
}


//spawn_task�ļ�ʵ��
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

//ATM�߼���ļ�ʵ��
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