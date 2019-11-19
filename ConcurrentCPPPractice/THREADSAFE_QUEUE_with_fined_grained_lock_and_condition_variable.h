//  可上锁和等待的线程安全队列(无界队列)
//  线程可以持续向队列中添加元素
//  THREADSAFE_QUEUE_with_fined_grained_lock_and_condition_variable.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/19.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef THREADSAFE_QUEUE_with_fined_grained_lock_and_condition_variable_h
#define THREADSAFE_QUEUE_with_fined_grained_lock_and_condition_variable_h

template<typename T>
class threadsafe_queue{
    
private:
    struct node{
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;
    std::condition_variable data_cond;
    
    node* get_tail(){
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    
    std::unique_ptr<node> pop_head(){ //弹出头部的真正操作
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
    
    std::unique_lock<std::mutex> wait_for_data(){ //等待队列中有数据弹出
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_cond.wait(head_lock,[&]{return head.get()!=get_tail();});
        return std::move(head_lock); //将锁的实例返回给调用者
    }
    
    std::unique_ptr<node> wait_pop_head(){
        //这就需要确保同一个锁在执行与wait_pop_head()重载的相关操作时，已持有锁
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }
    
    std::unique_ptr<node> wait_pop_head(T& value){
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value = std::move(*head->data);
        return pop_head();
    }
    
    std::unique_ptr<node> try_pop_head(){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail()){
            return std::unique_ptr<node>();
        }
        return pop_head();
        
    }
    
    std::unique_ptr<node> try_pop_head(T& val){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail()){
            return std::unique_ptr<node>();
        }
        val = std::move(*head->data);
        return pop_head();
    }
    
public:
    threadsafe_queue():head(new node),tail(head.get()){}
    threadsafe_queue(const threadsafe_queue& other) = delete;
    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;
    
    std::shared_ptr<T> try_pop(){
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head?old_head->data:std::shared_ptr<T>();
    }
    
    bool try_pop(T& val){
        std::unique_ptr<node> old_head = try_pop_head(val);
        return old_head;
    }
    
    std::shared_ptr<T> wait_and_pop(){
        std::unique_ptr<node> const old_head = wait_pop_head();
        return old_head->data;
    }
    
    void wait_and_pop(T& val){
        std::unique_ptr<node> const old_head = wait_pop_head(val);
    }
    
    void push(T new_value){
        std::shared_ptr<T> new_data(std::make_shared(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        {
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            
            node* const new_tail = p.get();
            tail->data = new_data;
            tail->next = std::move(p);
            tail = new_tail;
            
        }
        data_cond.notify_one();
    }
    
    void empty(){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        return (head.get()==get_tail());
    }
    
};


#endif /* THREADSAFE_QUEUE_with_fined_grained_lock_and_condition_variable_h */
