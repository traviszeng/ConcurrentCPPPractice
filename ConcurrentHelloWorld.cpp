#include<iostream>
#include<thread>
using namespace std;
void hello(){

    std::cout<<"Hello concurrent world."<<endl;
}
int main(){
    std::thread t(hello);
    t.join(); //ensure the main thread wait for the t thread
}

//in linux needs to compile as "g++ -std=c++11 -o hello ConcurrentHelloWorld.cpp -lpthread"