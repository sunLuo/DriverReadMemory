// ConsoleReadMemory.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "KInterface.h"

int main()
{
    std::cout << "Hello World!\n";
    uintptr_t addr = 0;
    addr = ClientAddress + 0x205F148;
    // 打印十六进制 (大写, 带 0x 前缀)
    printf("ClientAddress (Hex, uppercase): 0x%llX\n", ClientAddress);
    printf("addr1: 0x%llX\n", addr);
    // --- 读取一个 QWORD / DWORD64 (8字节) ---
    addr = Memory.Read<DWORD64>(addr);
    std::cout << "Read QWORD: 0x" << std::hex << addr << std::dec << std::endl;
    addr += 0x50;
    addr = Memory.Read<DWORD64>(addr);
    std::cout << "Read QWORD: 0x" << std::hex << addr << std::dec << std::endl;
    addr += 0x238;
    addr = Memory.Read<DWORD64>(addr);
    std::cout << "Read QWORD: 0x" << std::hex << addr << std::dec << std::endl;
    addr += 0x08;
    addr = Memory.Read<DWORD64>(addr);
    std::cout << "Read QWORD: 0x" << std::hex << addr << std::dec << std::endl;
    addr += 0x58 + 0x04;
    float armor = 0.0f;
    armor = Memory.Read<float>(addr);
    std::cout << "Armor: " << armor << std::endl;
    // 等待用户按键退出
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore(); // 忽略换行符
    std::cin.get();

    return 0;
}


