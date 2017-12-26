#include <Windows.h>
#include <iostream>

#define IOCTL_TEST\
	CTL_CODE(40000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

const WCHAR fileName[] = L"\\\\.\\test";

int main() {
	HANDLE device;
	const char *msg = "Hello from userland.";
	DWORD bytesRead;
	char ReadBuffer[50] = { 0 };

	device = CreateFileW(fileName,
		GENERIC_WRITE|GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (device == INVALID_HANDLE_VALUE) {
		std::cout << "Could not open handle" << std::endl;
		return 1;
	}

	DeviceIoControl(device, IOCTL_TEST, (char*)(msg), strlen(msg), ReadBuffer, sizeof(ReadBuffer), &bytesRead, NULL);
	printf("Kernel: %s\n", ReadBuffer);
	printf("Bytes: %d\n", bytesRead);

	CloseHandle(device);

	return 0;
}