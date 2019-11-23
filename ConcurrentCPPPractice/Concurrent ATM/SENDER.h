//  sender类
//  只能对已推送到队列中的消息进行包装。 对sender实例的拷贝，只是拷贝了指向队列的指针，而非队列本身。
//  SENDER.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/23.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef SENDER_h
#define SENDER_h

namespace messaging {
class sender{
    queue *q;   //sender是一个队列指针的包装类
public:
    sender():q(nullptr){}   //sender无队列（默认构造函数）
    explicit sender(queue *q_):q(q_){}  //从指向队列的指针进行构造
    
    template<typename Message>
    void send(Message const& msg){
        if(q){
            q->push(msg);   //将发送的消息推送到队列
        }
    }
};
}

#endif /* SENDER_h */
