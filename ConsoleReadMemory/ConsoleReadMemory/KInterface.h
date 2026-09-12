#pragma once

#ifndef _KINTERFACE_H_
#define _KINTERFACE_H_

#pragma warning (disable: 4302 4311 4244 4700 )

#include <Windows.h>
#include <apiquery2.h>
#include <string>

#define IO_GET_ID_REQUEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6210, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6211, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6212, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_GET_MODULE_REQUEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6213, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProcessId; //target process id
	ULONG_PTR Address; // address of memory to start reading from
	PVOID pBuff; // return value
	ULONG Size; // size of memory to read
} KERNEL_READ_REQUEST, * PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	ULONG ProcessId; //target process id
	ULONG_PTR Address; // address of memory to start reading from
	PVOID pBuff; // return value
	ULONG Size; // size of memory to read
} KERNEL_WRITE_REQUEST, * PKERNEL_WRITE_REQUEST;

class KInterface
{
private:
	HANDLE hDriver; // Handle to driver
	DWORD ProcessID = 0;


public:

	// Initializer
	KInterface(LPCSTR RegistryPath)
	{
		hDriver = CreateFileA(RegistryPath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);

		ProcessID = Process();
		std::cout << "ProcessID: " << ProcessID << std::endl;
	}

	DWORD Process()
	{
		if (ProcessID)
			return ProcessID;

		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		ULONG Id = 0;
		DWORD Bytes = 0;

		if (DeviceIoControl(hDriver, IO_GET_ID_REQUEST, &Id, sizeof(Id), &Id, sizeof(Id), &Bytes, NULL))
			return Id;
		else
			return false;
	}

	DWORD64 GetClientModule()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		DWORD64 Address = 0;
		DWORD Bytes = 0;

		if (DeviceIoControl(hDriver, IO_GET_MODULE_REQUEST, &Address, sizeof(Address),
			&Address, sizeof(Address), &Bytes, NULL))
			return Address;
		else
			return false;
	}

	template <typename type>
	type Read(ULONG_PTR ReadAddress, SIZE_T Size = sizeof(type))
	{
		//type cData;
		if (hDriver == INVALID_HANDLE_VALUE)
			return (type)false;

		//DWORD Return, Bytes;
		KERNEL_READ_REQUEST ReadRequest;

		ReadRequest.ProcessId = ProcessID;
		ReadRequest.Address = ReadAddress;
		//ReadRequest.pBuff = &cData;
		ReadRequest.Size = Size;

		if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest, sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), 0, 0))
			return  *(type*)&ReadRequest.pBuff;

		return (type)false;
	}

	
	/*bool Write(ULONG WriteAddress, ULONG WriteValue, SIZE_T WriteSize)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;
		DWORD Bytes;

		KERNEL_WRITE_REQUEST  WriteRequest;
		WriteRequest.ProcessId = ProcessID;
		WriteRequest.Address = WriteAddress;
		WriteRequest.pBuff = &WriteValue;
		WriteRequest.Size = WriteSize;

		if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteRequest, sizeof(WriteRequest),
			0, 0, &Bytes, NULL))
			return true;
		else
			return false;
	}*/
};

static KInterface Memory(R"(\\.\kbotl)");

static DWORD64 ClientAddress = Memory.GetClientModule();

#endif //!_KINTERFACE_H_