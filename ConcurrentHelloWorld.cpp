#include<algorithm>
#include<vector>
#include<functional>
#include<mutex>
#include<iostream>
#include<thread>
using namespace std;
void hello(){

    std::cout<<"Hello concurrent world."<<endl;
}

//lock_guard demo
std::mutex my_lock;

void add(int &num,int &sum){
    std::lock_guard<mutex> lock(my_lock);
    while(1){
        if(num<100){
            num+=1;
            sum+=num;
            std::cout<<std::this_thread::get_id()<<endl;
            std::cout<<"num:"<<num<<endl;
            std::cout<<"sum:"<<sum<<endl;
        }
        else{
            break;
        }
    }

}
int main(){
    std::thread t(hello);
    t.join(); //ensure the main thread wait for the t thread

    int sum = 0;
    int num = 0;
    std::vector<thread> ver; //用以保存线程的vector
    for(int i = 0;i<20;i++){
        std::thread t = std::thread(add,std::ref(num),std::ref(sum));
        ver.emplace_back(std::move(t)); //保存线程
    }
    std::for_each(ver.begin(),ver.end(),std::mem_fn(&std::thread::join));
    std::cout<<std::this_thread::get_id()<<endl;
    std::cout << sum << std::endl;
	//system("pause");
}

//in linux needs to compile as "g++ -std=c++11 -o hello ConcurrentHelloWorld.cpp -lpthread"