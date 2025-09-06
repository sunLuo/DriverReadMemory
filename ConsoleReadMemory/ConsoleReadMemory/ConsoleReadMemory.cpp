// ConsoleReadMemory.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "KInterface.h"

int main()
{
    std::cout << "Hello World!\n";
    DWORD processId = 0;      // 32-bit Process ID
    DWORD64 memoryAddress = 0; // 64-bit Memory Address

    // --- 输入进程ID (十进制) ---
    std::cout << "Enter Process ID (PID) [Decimal]: ";
    std::cin >> processId;

    // --- 输入内存地址 (十六进制) ---
    std::cout << "Enter Memory Address [Hexadecimal, e.g., 7FF7A8B00000 or 0x7FF7A8B00000]: ";
    std::cin >> std::hex >> memoryAddress; // std::hex 设置后续整数输入为十六进制
    // 打印十六进制 (大写, 带 0x 前缀)
    printf("Memory Address (Hex, uppercase): 0x%llX\n", memoryAddress);
    // --- 读取一个 QWORD / DWORD64 (8字节) ---
    DWORD64 qwordValue = Memory.Read<DWORD64>(memoryAddress, processId);
    std::cout << "Read QWORD: 0x" << std::hex << qwordValue << std::dec << std::endl;
    // 等待用户按键退出
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore(); // 忽略换行符
    std::cin.get();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
