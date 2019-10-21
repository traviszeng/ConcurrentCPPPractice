#include<iostream>
#include<thread>
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

int main(){
    task f;
    std::thread t(f);
    //t.detach(); //main thread will not wait for t thread to end
    t.join(); //main thread waits for t to end

}
