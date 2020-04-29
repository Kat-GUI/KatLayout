#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include"../src/Widget.h"
#include<iostream>
//using namespace DOM;
Margin* window;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void test2(){

}
using namespace std;

int main()
{
    window=new Margin(10);
    auto f = constraint(Limit(0,400),Limit(0,400),new Fixed( Fixed::infinity,100));
    window->setChild(f);
    //    auto g = new Grid(2,1);
//    auto r2 = new Row(100);
//    auto r = new Row(100);
//    r->addChild(new Fixed(100,100));
//    r->addChild(new Fixed(100,100));
//    r->addChild(new Fixed(100,100));
//    r->addChild(new Fixed(100,100));
//    r->addChild(new Fixed(100,100));
//    r->squeezeBack=[&](std::shared_ptr<Layout> layout){
//        r2->removeChild(layout);
//        std::cout<<"out"<<std::endl;
//    };
//    r->squeezeOut=[&](LayoutPtr layout){
//        r2->addChild(layout);
//        std::cout<<"back"<<std::endl;
//    };
//    g->addChild(0,0,r);
//    g->addChild(1,0,r2);
//    window->setChild(g);

//    auto w = new WarpPanel(Direction::Vertical);
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    w->addChild(new Fixed(200,100));
//    window->setChild(w);

//    auto g = new Grid({0.5,0.25,0.15,0.1},{0.5,0.25,0.15,0.1});
//    window->setChild(g);
//    g->addChild(0,0,new Margin(10));
//    g->addChild(1,2,2,2,new Margin(10));
//    auto m = new Margin(10);
//    window->setChild(m);
//    auto c = new ExtendColumn(100);
//
//    auto c2=new Column(100);
//    c2->addChild(new Fixed(Horizontal::right,Vertical::bottom,10,10,70,70,10,10));
//    c2->addChild(new Margin(10,Limit(100,150)));
//
//
//    auto d = new Dynamic();
//    d->addChild(new Margin(10,Limit(90,150)));
//    d->addChild(new Fixed(80,80));
//    d->addChild(new Fixed(30,80));
//
//    auto d2 = new Dynamic();
//    d2->addChild(new Margin(10,Limit(90,120)));
//    d2->addChild(new Fixed(80,80));
//    d2->addChild(new Fixed(30,80));
//    c->addChild(d);
//    c->addChild(d2);
//
//    auto d3 = new Dynamic();
//    d3->addChild(new Margin(10,Limit(90,150)));
//    d3->addChild(new Fixed(80,80));
//    d3->addChild(new Fixed(30,80));
//
//    auto d4 = new Dynamic();
//    d4->addChild(new Margin(10,Limit(90,120)));
//    d4->addChild(new Fixed(80,80));
//    d4->addChild(new Fixed(30,80));
//    c2->addChild(d3);
//    c2->addChild(d4);
//    c->addChild(c2);
//
//    m->setChild(c);
//
//    c->addChild(new Fixed(Horizontal::right,Vertical::bottom,10,10,70,70,10,10));
//    c->addChild(new Margin(10,Limit(100,150)));

    //    c->addChild(new Margin(10,Limit(50,100)));
//    c->addChild(new Margin(10,Limit(100,300)));

//    c->addChild(new Margin(10,Limit(100,200)));
//    c->addChild(new Margin(10,Limit(100,200)));
//    c->addChild(new Margin(10,Limit(100,200)));
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wc = { };
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground  = CreateSolidBrush(RGB(220,220,220));

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"Learn to Program Windows",    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style
            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
            NULL,       // Parent window
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }


    ShowWindow(hwnd, SW_SHOW);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            testHdc=hdc;
            LPRECT rect=new tagRECT();
            GetClientRect(hwnd,rect);
            Layout::Region anchor;
            anchor.l=0;
            anchor.t=0;
            anchor.w=rect->right;
            anchor.h=rect->bottom;
            anchor.r=anchor.w;
            anchor.b=anchor.h;
            window->calcuRegion(anchor);
            EndPaint(hwnd, &ps);
        }
            return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}