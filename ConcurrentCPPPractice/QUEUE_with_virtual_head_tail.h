//
//  QUEUE_with_virtual_head_tail.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/17.
//  Copyright © 2019 travis. All rights reserved.
//  拥有虚拟头尾节点的queue 头尾分离

#ifndef QUEUE_with_virtual_head_tail_h
#define QUEUE_with_virtual_head_tail_h

template<typename T>
class queue{
private:
    struct node{
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
        
    };
    
    std::unique_ptr<node> head;
    node* tail;
public:
    queue():head(new node),tail(head.get()){}
    queue(const queue& other) = delete;
    queue& operator=(const queue& other) = delete;
    
    std::shared_ptr<T> try_pop(){
        if(head.get()==tail){
            return std::shared_ptr<T>();
        }
        
        std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    
    void push(T new_val){
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_val))); //避免二次内存分配
        
        std::unique_ptr<node> p(new node); //创建的新节点成为虚拟节点
        tail->data = new_data; //将new_val的副本赋给之前的虚拟节点
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};

#endif /* QUEUE_with_virtual_head_tail_h */
