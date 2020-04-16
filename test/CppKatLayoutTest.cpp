#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include"../src/Widget.h"
#include<iostream>
using namespace DOM;
Widget window;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main()
{
    window.x.head=10;
    window.y.head=10;
    window.id="window";
    Dynamic container;
    Widget exten,exten2,filler;
    container.x.head=5;
    container.x.tail=5;
    container.y.head=5;
    container.y.tail=5;
    window.child=&container;

    exten.id="exten";
    exten.x.body=0;
    exten.y.body=0;
    exten.x.extended=true;
    exten.y.extended=true;
    container.candidate.push_back(std::make_pair(&exten,[](Size size){return size.width<400;}));
    container.candidate.push_back(std::make_pair(nullptr,[](Size size){return size.width>400&&size.width<600;}));
    container.candidate.push_back(std::make_pair(&exten,container.caseElse));
    exten2.id="exten2";
    exten2.x.head=10;
    exten2.x.tail=10;
    exten2.y.head=10;
    exten2.y.tail=10;
    exten2.x.body=0;
    exten2.y.body=0;
    exten2.x.extended=true;
    exten2.y.extended=true;
    exten.child=&exten2;

    filler.id="filler";
    filler.x.body=300;
    filler.y.body=300;
    filler.x.head=30;
    filler.y.head=30;
    filler.x.tail=50;
    filler.y.tail=50;
    exten2.child=&filler;

//    Widget child1,child2,center,filler;
//    child1.x.head=20;
//    child1.y.head=20;
//    child1.x.tail=20;
//    child1.y.tail=20;
//    child1.x.limit.min=300;
//    child1.y.limit.min=300;
//    child1.x.limit.max=600;
//    child1.y.limit.max=600;
//    window.child=&child1;
//
//    child2.x.body=300;
//    child2.y.body=300;
//    child1.child=&child2;
//
//    center.x.extended=true;
//    center.y.extended=true;
//    child2.child=&center;



    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    HINSTANCE hInstance = GetModuleHandle(0);
    WNDCLASS wc = { };
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);

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
            LPRECT rect=new tagRECT();
            GetClientRect(hwnd,rect);
            Widget* w=&window;
            w->region.x=0;
            w->region.y=0;
            w->region.w=rect->right;
            w->region.h=rect->bottom;
            w->render(hdc);
            EndPaint(hwnd, &ps);
        }
            return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}