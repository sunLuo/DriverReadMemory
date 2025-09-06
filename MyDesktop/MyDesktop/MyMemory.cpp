#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <stdio.h> // 用于 _stprintf_s

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;
int hp = 100;

// 为按钮定义一个唯一的控件ID
#define ID_BUTTON_SHOW_ADDRESS 1001
// --- 新增：为显示信息的编辑框定义控件ID ---
#define ID_EDIT_INFO 1002

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application dows not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600, 350,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // --- 创建用于显示信息的只读编辑框 ---
    HWND hEditInfo = CreateWindowEx(
        WS_EX_CLIENTEDGE, // 添加 3D 边框效果
        _T("EDIT"),       // EDIT 控件的预定义类名
        _T(""),           // 初始文本为空
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL,
        // 样式: 子窗口、可见、垂直滚动条、只读、多行、自动垂直滚动
        10, 60,           // 位置 (x, y)
        520, 150,         // 宽度, 高度
        hWnd,             // 父窗口
        (HMENU)ID_EDIT_INFO, // 控件ID
        hInstance,
        NULL
    );

    if (!hEditInfo)
    {
        MessageBox(hWnd, _T("Failed to create info edit control!"), _T("Error"), MB_ICONERROR);
    }
    else
    {
        // --- 在编辑框中设置初始文本 (进程ID) ---
        DWORD processId = GetCurrentProcessId();
        TCHAR initialText[512];
        _stprintf_s(initialText, 512,
            _T("Process Information:\r\n")
            _T("  - Process ID (PID): %u\r\n")
            _T("(You can select and copy this text! Click the button below to show 'hp' details.)"),
            processId);
        // 使用 WM_SETTEXT 消息设置编辑框文本
        SendMessage(hEditInfo, WM_SETTEXT, 0, (LPARAM)initialText);
        // ---
    }

    // --- 新增代码：创建按钮 ---
    // CreateWindow 的参数说明:
    // _T("BUTTON"): 标准按钮控件的预定义类名
    // _T("Show hInst Address"): 按钮上显示的文本
    // WS_TABSTOP | WS_VISIBLE | WS_CHILD: 样式 - 可接收Tab键焦点、可见、是子窗口(属于主窗口)
    // 200, 130: 按钮在父窗口内的位置 (x, y)
    // 100, 30: 按钮的宽度和高度
    // hWnd: 父窗口句柄 (我们的主窗口)
    // (HMENU)ID_BUTTON_SHOW_ADDRESS: 菜单/控件ID - 我们用它来识别按钮
    // hInstance: 应用程序实例句柄
    // NULL: 未使用
    HWND hButton = CreateWindow(
        _T("BUTTON"),  // 预定义按钮类
        _T("Show hp Address"), // 按钮文本
        WS_TABSTOP | WS_VISIBLE | WS_CHILD, // 样式
        215, 230, // 位置 (x, y)
        120, 30, // 宽度, 高度
        hWnd, // 父窗口
        (HMENU)ID_BUTTON_SHOW_ADDRESS, // 控件ID
        hInstance,
        NULL
    );

    if (!hButton)
    {
        MessageBox(hWnd, _T("Failed to create button!"), _T("Error"), MB_ICONERROR);
    }
    // --- 按钮创建结束 ---

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // Here your application is laid out.
        // For this introduction, we just print out "Hello, Windows desktop!"
        // in the top left corner.
        TextOut(hdc,
            5, 5,
            greeting, _tcslen(greeting));

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        // --- 新增消息处理：按钮点击 ---
    case WM_COMMAND:
    {
        // LOWORD(wParam) 包含了控件ID
        // HIWORD(wParam) 包含了通知码 (如 BN_CLICKED)
        // lParam 包含了控件窗口句柄
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        // 检查是否是我们的按钮发送的消息
        if (wmId == ID_BUTTON_SHOW_ADDRESS && wmEvent == BN_CLICKED)
        {
            // --- 获取要显示信息的编辑框句柄 ---
            HWND hEditInfo = GetDlgItem(hWnd, ID_EDIT_INFO);
            if (hEditInfo)
            {
                // --- 新增：显示 hp 的内存地址 ---
                // 首先，获取 hp 变量本身的地址 (即存储 hp 值的那个内存位置的地址)
                void* pAddressOfHp = &hp; // &hp 取变量 hp 的地址
                // 2. 获取当前进程的 ID
                DWORD processId = GetCurrentProcessId(); // 使用 Windows API

                TCHAR fullText[512];
                _stprintf_s(fullText, 512,
                    _T("Global variable 'hp' information:\r\n")
                    _T("  - Memory Address: 0x%p\r\n")
                    _T("  - Current Value: %d\r\n")
                    _T("Process Information:\r\n")
                    _T("  - Process ID (PID): %u"),
                    pAddressOfHp, hp, processId);

                // 更新编辑框中的文本
                SendMessage(hEditInfo, WM_SETTEXT, 0, (LPARAM)fullText);
            }
        }
        break;
    }
    // --- 按钮消息处理结束 ---
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}