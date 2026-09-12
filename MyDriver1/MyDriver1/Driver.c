#pragma warning (disable: 4100 4047 4024 4022)

#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>

#define IO_GET_ID_REQUEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6210, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6211, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6212, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_GET_MODULE_REQUEST  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6213, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

ULONG processId;
ULONG_PTR ClientAddress;  // 改为 ULONG_PTR
PDEVICE_OBJECT pDeviceObject;
UNICODE_STRING dev, dos; //driver registry paths
PLOAD_IMAGE_NOTIFY_ROUTINE  g_ImageNotifyCallback = NULL;

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

NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

NTSTATUS KernelReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	return MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(), TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS KernelWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	return MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process, TargetAddress, Size, KernelMode, &Bytes);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS UnloadDriver(PDRIVER_OBJECT DriverObject);
VOID ImageLoadCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp);

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = UnloadDriver;
	DbgPrintEx(0, 0, "lzhDriverEntry Started");

	NTSTATUS status = PsSetLoadImageNotifyRoutine(ImageLoadCallback);
	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID,
		DPFLTR_INFO_LEVEL,
		"lzh PsSetLoadImageNotifyRoutine status = 0x%08X\n",
		status
	);
	g_ImageNotifyCallback = ImageLoadCallback;

	RtlInitUnicodeString(&dev, L"\\Device\\kbotl");
	RtlInitUnicodeString(&dos, L"\\DosDevices\\kbotl");

	IoCreateDevice(DriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	IoCreateSymbolicLink(&dos, &dev);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

NTSTATUS UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	//DbgPrintEx(0, 0, "Unloaded");
	// 必须反注册，否则蓝屏
	if (g_ImageNotifyCallback)
	{
		PsRemoveLoadImageNotifyRoutine(ImageLoadCallback);
		g_ImageNotifyCallback = NULL;
	}
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(DriverObject->DeviceObject);

	return STATUS_SUCCESS;
}

//searches for lol
VOID  ImageLoadCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	// 安全校验指针有效性，防止蓝屏
	if (FullImageName == NULL || FullImageName->Buffer == NULL)
		return;
	// 直接打印模块完整路径、PID、加载基址
	WCHAR buf[128] = { 0 };
	ULONG copy = min(FullImageName->Length, sizeof(buf) - sizeof(WCHAR));
	RtlCopyMemory(buf, FullImageName->Buffer, copy);
	if (wcsstr(
		FullImageName->Buffer,
		L".exe"))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "lzhModuleLoad PID=%u path=%ws Base=%p\n",
			HandleToULong(ProcessId), buf, ImageInfo->ImageBase);
	}
	if (wcsstr(buf, L"\\Device\\HarddiskVolume4\\Program Files\\英雄联盟(26)\\Game\\League of Legends.exe"))
	{
		// 已经记录过，忽略后续匹配
		if (processId != 0)
		{
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID,
				DPFLTR_INFO_LEVEL,
				"lzh LOL already found, ignore PID=%lu\n",
				HandleToULong(ProcessId)
			);
			return;
		}
		DbgPrintEx(0, 0, "lzh Lol found\n");
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "lzhModuleLoad PID=%u path=%ws Base=%p\n",
			HandleToULong(ProcessId), buf, ImageInfo->ImageBase);
		ClientAddress = (ULONG_PTR)ImageInfo->ImageBase;
		processId = HandleToULong(ProcessId);
	}
}

NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG ByteIo = 0;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	//listen to user mode
	ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	if (ControlCode == IO_READ_REQUEST)
	{
		PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PKERNEL_READ_REQUEST ReadOutput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PEPROCESS Process;
		if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
		{
			KernelReadVirtualMemory(Process, ReadInput->Address, &ReadInput->pBuff, ReadInput->Size);
		}
		UNREFERENCED_PARAMETER(ReadOutput);
		DbgPrintEx(0, 0, "lzh Read Params:  %lu, %llX, %d\n", ReadInput->ProcessId, ReadInput->Address, ReadInput->Size);
		DbgPrintEx(0, 0, "lzh Value: %lu , %llX\n", ReadOutput->pBuff, ReadOutput->pBuff);

		Status = STATUS_SUCCESS;
		ByteIo = sizeof(KERNEL_READ_REQUEST);
	}
	else if (ControlCode == IO_WRITE_REQUEST)
	{
		PKERNEL_WRITE_REQUEST WriteInput = (PKERNEL_WRITE_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PEPROCESS Process;

		if (NT_SUCCESS(PsLookupProcessByProcessId(WriteInput->ProcessId, &Process)))
		{
			KernelWriteVirtualMemory(Process, &WriteInput->pBuff, WriteInput->Address, WriteInput->Size);
		}

		DbgPrintEx(0, 0, "lzh Write Params:  %lu, %llX \n", WriteInput->pBuff, WriteInput->Address);

		Status = STATUS_SUCCESS;
		ByteIo = sizeof(KERNEL_WRITE_REQUEST);
	}
	else if (ControlCode == IO_GET_ID_REQUEST)
	{
		PULONG_PTR OutPut = (PULONG_PTR)Irp->AssociatedIrp.SystemBuffer;
		*OutPut = processId;

		DbgPrintEx(0, 0, "lzh Pid: %u", processId);
		Status = STATUS_SUCCESS;
		ByteIo = sizeof(*OutPut);
	}
	else if (ControlCode == IO_GET_MODULE_REQUEST)
	{
		PULONGLONG OutPut = (PULONGLONG)Irp->AssociatedIrp.SystemBuffer;
		*OutPut = ClientAddress;

		DbgPrintEx(0, 0, "lzh ClientAddress: %llX\n", ClientAddress);
		Status = STATUS_SUCCESS;
		ByteIo = sizeof(*OutPut);
	}
	else
	{
		DbgPrintEx(0, 0, "lzh IoControl failed\n");
		Status = STATUS_INVALID_PARAMETER;
		ByteIo = 0;
	}

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = ByteIo;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	DbgPrintEx(0, 0, "lzh CreateCall\n");

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	DbgPrintEx(0, 0, "lzh CloseCall\n");

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}