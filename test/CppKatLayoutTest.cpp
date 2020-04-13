#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include"../src/Widget.h"
#include"TestWidget.h"
#include<iostream>
using namespace DOM;
testWidget window={left(10),top(10),width(1),height(1),child(
            new testWidget{left(10),top(10),right(10),bottom(10)}
        )};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main()
{
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
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
            *window.data.height=rect->bottom-100;
            *window.data.width=rect->right-100;
            window.render(hdc);
            window.resetRegion(Widget::Zero);
            EndPaint(hwnd, &ps);
            std::cout<<23333<<std::endl;
        }
            return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}