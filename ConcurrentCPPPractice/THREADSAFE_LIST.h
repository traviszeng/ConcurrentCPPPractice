//  支持迭代器的线程安全的链表
//  THREADSAFE_LIST.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/21.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef THREADSAFE_LIST_h
#define THREADSAFE_LIST_h

template<typename T>
class threadsafe_list{
    
    struct node{ //连标节点定义
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
        
        node():next(nullptr){} //默认构造函数
        node(T const& val): data(std::shared_ptr<T>(val)),next(nullptr){} //构造一个新的节点，在堆上分类内存对数据进行存储
    };
    
    node head;
public:
    threadsafe_list(){}
    ~threadsafe_list(){
        remove_if([](node const&){return true;});
    }
    
    threadsafe_list(threadsafe_list const& other)=delete;
    threadsafe_list& operator=(threadsafe_list const& other) = delete;
    
    void push_front(T const& value){
        std::unique_ptr<node> new_node(new node(value)); //构造一个新节点
        std::lock_guard<std::mutex> lk(head.m);
        new_node->next = std::move(head.next); //从头部插入
        head.next = std::move(new_node); //让头节点的next指向新节点
    }
    
    template<typename Function> //函数指针
    void for_each(Function f){ //传入一个b接受类型为T的值作为参数的通用函数或者传入一个有函数操作的类型对象
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m); //锁住head的互斥量
        while(node* const next = current->next.get()){ //直到next为空指针
            //安全的获取指向下一个节点的指针(使用get()获取，这是因为对这 个指针没有所有权)
            std::unique_lock<std::mutex> next_lk(next->m); //对需要指向的节点上锁
            lk.unlock(); //对上一个节点解锁
            f(*next->data); //调用指定函数
            current = next; //更新当前节点
            lk = std::move(next_lk); //将锁的所有权转移
            
        }
    }
    
    //find_first_if支持函数(谓词)在匹配的时候 返回true，在不匹配的时候返回false
    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p){ //14
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next = current->next.get()){
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            if(p(*next->data)){ //谓词匹配 判断当前元素是否符合匹配条件
                return next->data; //条件匹配时，只需要返回找到的数据，不继续查找
            }
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }
    
    template<typename Predicate>
    void remove_if(Predicate p){ //会改变链表结构，与for_each不同
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while(node* const next = current->next.get()){
            std::unique_lock<std::mutex> next_lk(next->m);
            if(p(*next->data)){ //谓词匹配时删除对应元素 并更新current->next
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lk.unlock();
            }   //20
            else{
                lk.unlock();    //不匹配时就是一个手递手的锁传递
                current = next;
                lk = std::move(next_lk);
            }
        }
    }
};


#endif /* THREADSAFE_LIST_h */
