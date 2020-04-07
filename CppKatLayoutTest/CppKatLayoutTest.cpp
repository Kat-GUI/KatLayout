// CppKatLayoutTest.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "CppKatLayoutTest.h"
#include "Layout.h"
#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CPPKATLAYOUTTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPPKATLAYOUTTEST));

    MSG msg;

    
    /*layout.right=new int(10);
    layout.bottom=new int(10);
    layout.width=new int(50);
    layout.height=new int(40);*/

    //layout.left=new int(10);
    //layout.top=new int(10);
    //layout.width=new int(50);
    //layout.height=new int(40);

    //layout.scale_right=new float(0);
    //layout.scale_bottom=new float(0);
    //layout.scale_width=new float(0.5);
    //layout.scale_top=new float(0);

    //layout.scale_right=new float(0.1);
    //layout.scale_bottom=new float(0.1);
    //layout.scale_left=new float(0.1);
    //layout.scale_top=new float(0.1);

    //layout.scale_bottom = new float(0);
    //layout.width = new int(50);
    //layout.scale_top = new float(0);

    //layout.width = new int(50);
    //layout.height = new int(50);

   
    //Layout layout, parent, l;
    //parent.region.x = 0;
    //parent.region.y = 0;
    //parent.region.w = 100;
    //parent.region.h = 100;
    //l.left = new int(10);
    //l.top = new int(10);
    //l.width = new int(20);
    //l.height = new int(20);
    //l.resetRegion(&parent);

    //layout.setNeighbor(&l, Layout::Edge::left);
    //layout.height = new int(50);
    //layout.right = new int(0);
    //layout.resetRegion(&parent);


    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPPKATLAYOUTTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CPPKATLAYOUTTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
HWND hWnd;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            //Layout parent;
            //parent.region.x = 0;
            //parent.region.y = 0;
            //LPRECT rect=new RECT;
            //GetClientRect(hWnd,rect);
            //parent.region.w = rect->right;
            //parent.region.h = rect->bottom;

            //Layout *layout=new Layout[10];
            ////layout[0].left=new int(200);
            ///*layout[0].top=new int(50);*/
            ///*layout[0].right=new int(200);*/
            ////layout[0].scale_width =new float(0.8);
            ////layout[0].scale_left=new float(0.2);
            //layout[0].pending_height=true;
            ////layout[0].right=new int(50);
            //layout[0].pending_width=true;
            //layout[0].setChild(&layout[1]);
            ////layout[0].setSmallerChild(&layout[2],[&](Size size){return size.width<600;});

            ////layout[1].bottom = new int(5);
            //layout[1].left = new int(5);
            ///*layout[1].top = new int(5);*/
            //layout[1].width = new int(300);
            //layout[1].height = new int(30);

            //layout[2].setNeighbor(&layout[1],Layout::Edge::left);
            //layout[2].height = new int(100);
            //layout[2].width = new int(100);

            //layout[3].setNeighbor(&layout[2], Layout::Edge::left);
            //layout[3].height = new int(200);
            //layout[3].width = new int(400);

            ////parent.setChild(&layout[0]);
            //layout[0].resetRegion(&parent);
            //layout[1].resetRegion(&layout[0]);
            //layout[2].resetRegion(&layout[0]);
            //layout[3].resetRegion(&layout[0]);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            //// TODO: 在此处添加使用 hdc 的任何绘图代码...
            //Layout *l=&layout[0];
            //Rectangle(hdc, l->region.x, l->region.y,l->region.x + l->region.w,l->region.y + l->region.h);
            //l = &layout[1];
            //if(l!=nullptr)Rectangle(hdc, l->region.x, l->region.y, l->region.x + l->region.w, l->region.y + l->region.h);
            //l = &layout[2];
            //if (l != nullptr)Rectangle(hdc, l->region.x, l->region.y, l->region.x + l->region.w, l->region.y + l->region.h);
            //l = &layout[3];
            //if (l != nullptr)Rectangle(hdc, l->region.x, l->region.y, l->region.x + l->region.w, l->region.y + l->region.h);



            




            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
