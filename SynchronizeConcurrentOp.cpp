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
