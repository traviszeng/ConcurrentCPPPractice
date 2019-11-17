//
//  SINGLE_THREAD_QUEUE.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/17.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef SINGLE_THREAD_QUEUE_h
#define SINGLE_THREAD_QUEUE_h

template<typename T>
class queue{
private:
    struct node{
        T data;
        std::unique_ptr<node> next;
        
        node(T data_){
            data = std::move(data_);
            next = nullptr;
        }
    };
    
    std::unique_ptr<node> head;
    node* tail;
    
public:
    queue(){}
    queue(const queue& other) = delete; //不允许定义拷贝构造
    queue& operator=(const queue& other) = delete; //不允许使用赋值运算
    
    std::shared_ptr<T> try_pop(){
        if(head==nullptr){
            return std::shared_ptr<T>();
        }
        
        std::shared_ptr<T> const res( std::make_shared<T>(std::move(head->data)));
        
        std::unique_ptr<node> old_head = std::move(head);
        
        head = std::move(old_head->next);
        
        return res;
    }
    
    void push(T new_val){
        std::unique_ptr<node> p(new node(std::move(new_val)));
        
        node* const new_tail = p.get(); //相当于将new_tail指向p指向的元素
        
        if(tail){
            tail->next = std::move(p);
        }
        else{//空队列
            head = std::move(p);
        }
        tail = new_tail;
    }
    
    
};


#endif /* SINGLE_THREAD_QUEUE_h */
