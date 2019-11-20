//
//  ConcurrentHelloWorld.cpp
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/13.
//  Copyright © 2019 travis. All rights reserved.
//



#include<algorithm>
#include<vector>
#include<functional>
#include<mutex>
#include<iostream>
#include<thread>
//#include"SINGLE_THREAD_QUEUE.h"
//#include"threadsafe_stack.h"
//#include"THREADSAFE_QUEUE_shared_ptr.h"
//#include"QUEUE_with_virtual_head_tail.h"
#include"THREADSAFE_QUEUE_with_fined_grained_lock.h"
#include"THREADSAFE_LOOKUP_TABLE.h"
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
