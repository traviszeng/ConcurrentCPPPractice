## C++ Concurrency In Action Note

staring at 2019-11-13


### 基于锁的并发数据结构

设计并发的数据结构，意味着多个线程可以并发地访问这个数据结构，线程可以对这个数据结构作相同或不同的操作。

如何保证数据结构是线程安全的：

1.确保无线程能看到数据结构的“不变量”破坏时的状态。

2.小心处理会引起条件竞争的结构，提供完整操作的函数，而非操作步骤。

3.注意数据结构的行为是否会产生异常，从而确保“不变量”

4.将死锁的概率降到最低，使用数据结构时，需要限制锁的范围，且避免嵌套锁的存在。

#### 线程安全的栈

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/threadsafe_stack.h)

线程安全的栈使用锁和互斥量来保护线程安全，但由于定期等待和检查empty()和pop()以及对于empty_stack异常进行关注会浪费栈的资源，可以考虑使用条件变量来异步通知。

#### 使用条件变量和锁的线程安全队列

线程安全队列需要实现的功能：

1.对整个队列的状态进行查询(empty()和size());

2.查询在队列中的各个元素(front()和back())；

3.修改队列的操作(push(),pop()和emplace())。也会遇到在固有接口上的条件竞争。
需要将front()和pop()合并成一个函数调用，就像之前在栈实现时合并top()和pop()一
样。

不同的是：当使用队列在多个线程中传递数据时，接收线程通常需要等待数据的压入。
这里提供pop()函数的两个变种：try_pop()和wait_and_pop()。
try_pop() ，尝试从队列中弹出数据，总会直接返回(当有失败时)，即使没有指可检索；
wait_and_pop()，将会等待有值可检索的时候才返回。

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_QUEUE_with_condition_variable.h)

源码中也存在着一些问题：

若得到notify one的线程在wait and pop中抛出异常，则其他线程将继续休眠，无法再次得到notify

方案一：

当这种情况是不可接受时，这里的调用就需要改成data cond.notify all()，这个函数将唤醒所有的工作线程，不过，当大多线程发现队列依旧是空时，又会耗费很多资源让线程重新进入睡眠状态。

方案二：

当有异常抛出的时候，让wait_and_pop()函数调用notify_one()，从而让个另一个线程可以去尝试索引存储的值。

方案三：
			
将 std::shared_ptr<> 的初始化过程移到push()中，并且存储 std::shared_ptr<> 实例，而非直接使用数据的值。

将 std::shared_ptr<> 拷贝到内部 std::queue<> 中，就不会抛出异常了，这样wait_and_pop()又是安全的了。
使用方案三进行改进见THREADSAFE_QUEUE_shared_ptr.h

#### 使用条件变量和锁的线程安全队列（改进）

直接将分配内存的部分放在push函数中，这样可以保证新的实例在分配结束时，不会被锁在push中。

因为内存分配操作的需要在性能上付出很高的代价(性能较低)，所以使用 std::shared_ptr<> 的方式对队列的性能有很大的提升，其减少了互斥量持有的时间，允许其他线程在分配内存的同时，对队列进行其他的操作。

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_QUEUE_shared_ptr.h)

#### 使用细粒度锁的线程安全队列

对于每个元素使用一个互斥量来保护，可以使用细粒度锁进行保护。

单线程版本的线程安全队列[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/SINGLE_THREAD_QUEUE.h)

通过添加虚拟头尾指针实现头尾分离，从而实现一定程度的并发。

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/QUEUE_with_virtual_head_tail.h)

基于虚拟指针的使用细粒度锁的线程安全队列：

分别对头和尾采用两个互斥量进行保护，可以更大程度的实现并发性能。

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_QUEUE_with_fined_grained_lock.h)

#### 使用细粒度锁和条件变量实现可等待的无界队列

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_QUEUE_with_fined_grained_lock_and_condition_variable.h)


#### 使用锁的线程安全查询表

查询表的常用基本功能如下：

1.添加一对“键值-数据” 

2.修改指定键值所对应的数据 

3.删除一组值 

4.通过给定键值，获取对应数据

5.判断是否为空

6.转换成std::map<>映射

需要使用到c++的boost库

Mac OS上使用brew的安装方法：

	brew install boost


	
[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_LOOKUP_TABLE.h)
	

#### 使用细粒度锁支持迭代器的线程安全的链表

满足条件的链表需要有以下功能：

1.向列表添加一个元素 

2.当某个条件满足时，就从链表中删除某个元素 

3.当某个条件满足时，从链表中查找某个元素 

4.当某个条件满足时，更新链表中的某个元素 

5.将当前容器中链表中的每个元素，复制到另一个容器中

[源码](https://github.com/traviszeng/ConcurrentCPPPractice/blob/master/ConcurrentCPPPractice/THREADSAFE_LIST.h)


