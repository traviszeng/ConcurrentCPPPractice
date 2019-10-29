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
