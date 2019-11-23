//  消息队列实现
//  可以将消息以指针(指向基类)的方式存储在列表中；指定消 息类型会由基类派生模板进行处理。推送包装类的构造实例，以及存储指向这个实例的指 针；弹出实例的时候，将会返回指向其的指针。因为message_base类没有任何成员函数，在 访问存储消息之前，弹出线程就需要将指针转为wrapped_message指针。
//  MESSAGE_QUEUE.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/23.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef MESSAGE_QUEUE_h
#define MESSAGE_QUEUE_h

#include<mutex>
#include<condition_variable>
#include<queue>
#include<memory>

namespace  messaging {

struct message_base{ //队列项的基础类
    virtual ~message_base(){}
};

template<typename Msg>
struct wrapped_message:message_base{    //每个消息类型都需要特化
    Msg contents;
    
    explicit wrapped_message(Msg const& contents_):contents(contents_){}
};

class queue{
    std::mutex m;
    std::condition_variable c;
    std::queue<std::shared_ptr<message_base>> q;    //实际存储指向的是message_base类指针的队列
    
public:
    
    template<typename T>
    void push(T const& msg){
        std::lock_guard<std::mutex> lk(m);
        q.push(std::make_shared<wrapped_message<T>>(msg)); //包装已传递的信息，存储指针
        c.notify_all();
    }
    
    std::shared_ptr<message_base> wait_and_pop(){
        std::unique_lock<std::mutex> lk(m);
        c.wait(lk,[&]{return !q.empty();}); //当队列为空的时候阻塞
        auto res = q.front();
        q.pop();
        return res;
    }
    
};


}



#endif /* MESSAGE_QUEUE_h */
