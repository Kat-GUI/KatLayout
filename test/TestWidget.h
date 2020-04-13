//
// Created by Yorkin on 2020/4/13.
//
#ifndef LAYOUTTEST_TESTWIDGET_H
#define LAYOUTTEST_TESTWIDGET_H
#include"../src/Widget.h"
#include<windows.h>
class testWidget:public Widget
{
    Layout* layout;
public:
    testWidget(DOM::initializer property):Widget::Widget(property){}
    void render(HDC hdc)
    {
        Rectangle(hdc,region.x,region.y,region.w,region.h);
        if(data.child!=nullptr)
        {
            data.child->resetRegion(this);
            ((testWidget*)data.child)->render(hdc);
        }

    }
};
#endif //LAYOUTTEST_TESTWIDGET_H
