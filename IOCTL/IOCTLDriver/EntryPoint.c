#include <ntddk.h>
#include "debug.h"

#define IOCTL_TEST\
	CTL_CODE(40000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

const WCHAR DeviceNameBuffer[] = L"\\Device\\test";
const WCHAR SymLinkBuffer[] = L"\\DosDevices\\test";
PDEVICE_OBJECT MyDevice;

NTSTATUS FUNC_IRP_MJ_CREATE(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(Irp);

	PRINT("=> _IRP_MJ_CREATE called");
	return STATUS_SUCCESS;
}

NTSTATUS FUNC_IRP_MJ_CLOSE(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(Irp);

	PRINT("=> _IRP_MJ_CLOSE called");
	return STATUS_SUCCESS;
}

NTSTATUS FUNC_IRP_MJ_DEVICE_CONTROL(PDEVICE_OBJECT DriverObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DriverObject);

	PIO_STACK_LOCATION pIoStackLocation;
	PCHAR msg = "Hello World - from kernel!";
	PVOID pBuf = Irp->AssociatedIrp.SystemBuffer;

	PRINT("=> _IRP_DEVICE_CONTROL called");

	pIoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	if(pIoStackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_TEST)
	{
		PRINT("=> IOCTL_TEST event");
		PRINT("=> Message received : %s", pBuf);

		RtlZeroMemory(pBuf, pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength);
		RtlCopyMemory(pBuf, msg, strlen(msg));

		Irp->IoStatus.Information = strlen(msg);
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

VOID OnUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING slName;
	RtlInitUnicodeString(&slName, SymLinkBuffer);

	IoDeleteSymbolicLink(&slName);
	IoDeleteDevice(DriverObject->DeviceObject);
	PRINT("=> OnUnload called");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	UNICODE_STRING slName, dName;

	PRINT("=> DriverEntry called");

	RtlInitUnicodeString(&slName, SymLinkBuffer);
	RtlInitUnicodeString(&dName, DeviceNameBuffer);

	status = IoCreateDevice(DriverObject,
		0,
		&dName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_UNKNOWN,
		FALSE,
		&MyDevice);

	if (!NT_SUCCESS(status)) {
		PRINT("=> IoCreateDevice failed");
		return status;
	}

	status = IoCreateSymbolicLink(&slName, &dName);

	if (!NT_SUCCESS(status)) {
		PRINT("=> IoCreateSymbolicLink failed");
		return status;
	}

	DriverObject->DriverUnload = OnUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = FUNC_IRP_MJ_CREATE;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = FUNC_IRP_MJ_CLOSE;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FUNC_IRP_MJ_DEVICE_CONTROL;


	return STATUS_SUCCESS;
}