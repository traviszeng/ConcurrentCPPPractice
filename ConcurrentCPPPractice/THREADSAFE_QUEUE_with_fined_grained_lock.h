//
//  THREADSAFE_QUEUE_with_fined_grained_lock.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/17.
//  Copyright © 2019 travis. All rights reserved.
//  细粒度锁版本的线程安全队列

#ifndef THREADSAFE_QUEUE_with_fined_grained_lock_h
#define THREADSAFE_QUEUE_with_fined_grained_lock_h

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
    
    node* get_tail(){
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }
    

//    std::unique_ptr<node> pop_head(){  //有缺陷的实现
//        node* const old_tail = get_tail(); //在head_mutex范围外获取旧尾节点的值
//        std::lock_guard<std::mutex> head_lock(head_mutex);
//        if(head.get()==old_tail){
//            return nullptr;
//        }
//        std::unique_ptr<node> old_head = std::move(head);
//        head = std::move(old_head->next);
//        return old_head;
//    }
    
    
    std::unique_ptr<node> pop_head(){
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail()){
            return nullptr;
        }
        
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
    
public:
    threadsafe_queue():head(new node),tail(head.get()){}
    
    threadsafe_queue(const threadsafe_queue& other) = delete;
    threadsafe_queue& operator=(const threadsafe_queue& other) = delete;
    
    std::shared_ptr<T> try_pop(){
        std::unique_ptr<T> res = pop_head();
        return res;
    }
    
    void push(T new_val){
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_val)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        tail->next = std::move(p); //此时p已释放
        tail = new_tail;
    }
    
};


#endif /* THREADSAFE_QUEUE_with_fined_grained_lock_h */
