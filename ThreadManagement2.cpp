#include<iostream>
#include<thread>
#include<vector>
#include<algorithm>
using namespace std;
class task{

    public:
        void operator()() const{
            cout<<"background task running"<<endl;
        }
};

struct func{
    int &i;
    func(int &i_):i(i_){}
    void operator()(){
        for(unsigned j = 0;j<1000000;++j){
            cout<<i;
        }
    }

};

void oops(){

    int some_local_state=0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach(); // will use destroyed some_local_state error!
}
/**
 * using RAII to wait for the thread to be finished
 * 
*/
class thread_guard{
    std::thread &t;
    public:
        explicit thread_guard(std::thread& t_):t(t_){}

        ~thread_guard(){
            if(t.joinable()){ //thread can only joined once
                t.join();
            }

        }
        thread_guard(thread_guard const&)=delete;
        thread_guard& operator=(thread_guard const&)=delete;

};


/**
 * after f() finished g will be first to destroyed
 * t will be joined to the original thread
*/
void f(){
    int flag = 0;
    func my_func(flag);
    std::thread t(my_func);
    thread_guard g(t);
    cout<<"do something"<<endl; 
} 

void some_function() {
	cout << "some function" << endl;
}
void some_other_function() {
	cout << "some other function" << endl;
}

//使用move 新线程直接传递到scoped_thread中
class scoped_thread {
	std::thread t;
public:
	explicit scoped_thread(std::thread t_) :t(std::move(t_)) {
		if (!t.joinable()) throw std::logic_error("No thread");
	}

	~scoped_thread() {
		t.join();
	}

	scoped_thread(scoped_thread const&) = delete;
	scoped_thread& operator=(scoped_thread const&) = delete;
};

/**
批量产生线程
*/
void do_work(unsigned id) {
	cout << id << endl;
}
void batch_f() {
	std::vector<std::thread> threads;
	for (unsigned i = 0; i < 20; ++i)
	{
		threads.push_back(std::thread(do_work, i)); // 产生线程
	}
	std::for_each(threads.begin(), threads.end(),
		std::mem_fn(&std::thread::join)); // 对每个线程调用join()

}

/**
并行版std::accumulate 累加一个容器里面的所有值结果
*/
#include<numeric>
template<typename Iterator,typename T>
struct accumulate_block {
	void operator()(Iterator first, Iterator last, T& result) {
		result = std::accumulate
	}
};

template<typename Iterator, typename T>
T parallel_accmulate(Iterator first, Iterator last, T init) {
	//并行数据长度
	unsigned long const length = std::distance(first, last);
	
	if (!length)
		return init;

	unsigned long const min_per_thread = 25; 
	unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread; // 要用范围内元素的总数量除以线程(块)中最小任务数，从而确定启动线程的最大数量
	unsigned long const hardware_threads=std::thread::hardware_concurrency(); //返回能同时并发在一个程序中的线程数
	unsigned long const num_threads = // 
		std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

	unsigned long const block_size = length / num_threads; //每个线程中处理的元素数量

	std::vector<T> results(num_threads);
	std::vector<std::thread> threads(num_threads - 1); //系统已有主线程 因此需要少一个

	Iterator block_start = first;
	for (unsigned long i = 0; i < (num_threads - 1); ++i)
	{
		Iterator block_end = block_start;
		std::advance(block_end, block_size); // 向前移动block_size
		threads[i] = std::thread( // 7
			accumulate_block<Iterator, T>(),
			block_start, block_end, std::ref(results[i]));
		block_start = block_end; // 8
	}
	accumulate_block<Iterator, T>()(
		block_start, last, results[num_threads - 1]); // 最后一个block可能不满block_size个元素
	std::for_each(threads.begin(), threads.end(),
		std::mem_fn(&std::thread::join)); // 10
	return std::accumulate(results.begin(), results.end(), init); // 最终的累加
}

std::thread::id master_thread = std::this_thread::get_id();
void some_core_part_of_algorithm()
{
	if (std::this_thread::get_id() == master_thread)
	{
		cout << "It's master thread." << endl;
	}
	else {
		cout << "It's child thread." << endl;
	}
}



int main(){
	//std::thread::id master_thread = std::this_thread::get_id();
	some_core_part_of_algorithm();
    task f;
    std::thread t(f);
    //t.detach(); //main thread will not wait for t thread to end
    t.join(); //main thread waits for t to end
	
	std::thread t1(some_function); // 1
	std::thread t2 = std::move(t1); // 2
	t1 = std::thread(some_other_function); // 3
	std::thread t3; // 4
	t3 = std::move(t2); // 5
	//std::terminate();
	//t1 = std::move(t3); // 6 赋值操作将使程序崩溃 不可通过赋新值来抛弃一个进程
}
