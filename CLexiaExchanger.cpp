#include "CLexiaExchanger.h"
#include "dllmain.h"
#include <cfgmgr32.h>
#include <strsafe.h>
#include <iostream>
#include <ctime>

#define IOCTL_SEND_COMMAND 0x22200C
#define IOCTL_GET_RESULT 0x222010

#define MAX_DEVPATH_LENGTH 256
DEFINE_GUID(GUID_DEVINTERFACE_LEXIA, 0x75A835F4L, 0xD77D, 0x4402, 0x85, 0x85, 0xC4, 0x22, 0x47, 0xF2, 0x5B, 0x76);


LEXIA_STATUS CLexiaExchanger::GetDevicePath(_In_  LPGUID InterfaceGuid, _Out_writes_z_(BufLen) PWCHAR DevicePath, _In_ size_t BufLen)
{
	CONFIGRET cr = CR_DEFAULT;
	PWSTR deviceInterfaceList = NULL;
	ULONG deviceInterfaceListLength = 0;
	PWSTR nextInterface;
	HRESULT hr = E_FAIL;
	LEXIA_STATUS bRet = LEXIA_FAILED;

	cr = CM_Get_Device_Interface_List_Size(&deviceInterfaceListLength, InterfaceGuid, NULL, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

	if (cr != CR_SUCCESS)
	{
		LexiaLog("Error 0x%x retrieving device interface list size.", cr);
		goto clean0;
	}
	if (deviceInterfaceListLength <= 1)
	{
		LexiaLog("Error: No active device interfaces found.");
		cr = CR_DEFAULT;
		goto clean0;
	}
	deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
	if (deviceInterfaceList == NULL)
	{
		LexiaLog("Error allocating memory for device interface list.");
		cr = CR_DEFAULT;
		goto clean0;
	}
	ZeroMemory(deviceInterfaceList, deviceInterfaceListLength * sizeof(WCHAR));

	cr = CM_Get_Device_Interface_List(InterfaceGuid, NULL, deviceInterfaceList, deviceInterfaceListLength, CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

	if (cr != CR_SUCCESS)
	{
		LexiaLog("Error 0x%x retrieving device interface list.", cr);
		goto clean0;
	}

	nextInterface = deviceInterfaceList + wcslen(deviceInterfaceList) + 1;

	if (*nextInterface != UNICODE_NULL)
	{
		LexiaLog("Warning: More than one device interface instance found. Selecting first matching device.");
	}

	hr = StringCchCopy(DevicePath, BufLen, deviceInterfaceList);
	if (FAILED(hr))
	{
		LexiaLog("Error: StringCchCopy failed with HRESULT 0x%x", hr);
		goto clean0;
	}

clean0:
	if (deviceInterfaceList != NULL)
	{
		free(deviceInterfaceList);
	}
	if (CR_SUCCESS == cr)
	{
		bRet = LEXIA_NOERROR;
	}
	return bRet;
}

CLexiaExchanger::CLexiaExchanger()
{
	Device = INVALID_HANDLE_VALUE;
}
LEXIA_STATUS CLexiaExchanger::Disconnect()
{
	if (Device != INVALID_HANDLE_VALUE)
	{
		if (CloseHandle(Device))
		{
			LexiaLog("Device closed");
			Device = INVALID_HANDLE_VALUE;
			return LEXIA_NOERROR;
		}
	}
	LexiaLog("Device close failed");
	return LEXIA_FAILED;
}

LEXIA_STATUS CLexiaExchanger::Connect()
{
	WCHAR completeDeviceName[MAX_DEVPATH_LENGTH];

	if (Device != INVALID_HANDLE_VALUE)
	{
		LexiaLog("Already in use");
		return LEXIA_IN_USE;
	}

	if (GetDevicePath((LPGUID)& GUID_DEVINTERFACE_LEXIA, completeDeviceName, sizeof(completeDeviceName) / sizeof(completeDeviceName[0])) != LEXIA_NOERROR)
	{
		LexiaLog("Lexia not found");
		return  LEXIA_NOT_CONNECTED;
	}

	LexiaLog("DeviceName: %S", completeDeviceName);

	Device = CreateFile(completeDeviceName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (Device == INVALID_HANDLE_VALUE)
	{
		LexiaLog("Failed to open the device, error - %d", GetLastError());
		return LEXIA_FAILED;
	}
	else
	{
		LexiaLog("Device opened successfully.");
		return LEXIA_NOERROR;
	}
}
LEXIA_STATUS CLexiaExchanger::SendReceive(void* in, size_t in_len, void* out, size_t* out_len)
{
	if (Device == INVALID_HANDLE_VALUE)
	{
		return LEXIA_NOT_CONNECTED;
	}
	BOOL bStatus = false;
	ULONG ulReturnedLength = 0;
	char tmp_buf[8];
	LexiaLog("Binary send: %s", BytesToCharArray((UCHAR*)in, in_len));
	bStatus = DeviceIoControl(Device, IOCTL_SEND_COMMAND, in, in_len, tmp_buf, 8, &ulReturnedLength, NULL);
	//00 00 01 00 44 00 00 00
	// 01 - data available
	// 44 - out size
	LexiaLog("Status mesg: %s", BytesToCharArray((UCHAR*)tmp_buf, ulReturnedLength));
	if (ulReturnedLength >= 8 && bStatus)
	{
		int rcv_len = *((int*)& tmp_buf[4]);
		*(int*)tmp_buf = rcv_len;
		bStatus = DeviceIoControl(Device, IOCTL_GET_RESULT, tmp_buf, 4, out, rcv_len, &ulReturnedLength, NULL);
		if (bStatus && ulReturnedLength)
		{
			LexiaLog("Binary recv: %s", BytesToCharArray((UCHAR*)out, ulReturnedLength));
			*out_len = ulReturnedLength;
			return LEXIA_NOERROR;
		}
	}

	if (!bStatus)
	{
		LexiaLog("Ioctl failed with code %d", GetLastError());
		return LEXIA_IO_ERROR;
	}
	return LEXIA_FAILED;
}

CLexiaExchanger::~CLexiaExchanger()
{
	Disconnect();
}
