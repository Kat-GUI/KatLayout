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
    auto m = new Margin(10);
    window->setChild(m);
    auto c = new Column(100);
    auto d = new Dynamic();
    d->addChild(new Margin(10,Limit(90,350)));
    d->addChild(new Fixed(80,80));
    d->addChild(new Fixed(30,80));
    auto d2 = new Dynamic();
    d2->addChild(new Margin(10,Limit(90,350)));
    d2->addChild(new Fixed(80,80));
    d2->addChild(new Fixed(30,80));
    m->setChild(c);
    c->addChild(d);
    c->addChild(d2);
    c->addChild(new Fixed(Horizontal::right,Vertical::bottom,10,10,70,70,10,10));
    c->addChild(new Margin(10,Limit(100,200)));
    c->addChild(new Margin(10,Limit(100,200)));
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