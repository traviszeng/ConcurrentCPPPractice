//  receiver
//  RECEIVER.h
//  ConcurrentCPPPractice
//
//  Created by travis on 2019/11/23.
//  Copyright © 2019 travis. All rights reserved.
//

#ifndef RECEIVER_h
#define RECEIVER_h
#include"DISPATCH.h"

namespace messaging {
class receiver{
    queue q;    //接受者拥有对应队列
public:
    operator sender(){  //允许将类中队列隐式转化为一个sender队列
        return sender(&q);
    }
    
    dispatcher wait(){  //等待对队列进行调度
        return dispatcher(&q);
    }
};
}


#endif /* RECEIVER_h */
