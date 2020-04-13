//
// Created by Yorkin on 2020/4/13.
//
#ifndef LAYOUTTEST_MESSAGE_H
#define LAYOUTTEST_MESSAGE_H

enum Message{refersh};
class MessageReceiver{
public:
    virtual void process(Message msg)=0;
};


#endif //LAYOUTTEST_MESSAGE_H
