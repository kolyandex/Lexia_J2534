#include "pch.h"
#include "j2534.h"
#include "CTranslator.h"
#include <ctime>
#include <consoleapi3.h>
#include <iostream>

#define LOG_OUTPUT
char errorStr[] = "UNKNOWN_ERROR";
using namespace std;

#ifdef LEXIAJ2534_EXPORTS
#define LEXIAJ2534_API __declspec(dllexport)
#else
#define LEXIAJ2534_API __declspec(dllimport)
#endif

clock_t clk = clock();
CTranslator* translator = new CTranslator();

static char * hexstr = new char[8192];
static char* BytesToCharArray(UCHAR* data, UINT size)
{	
	UINT i;
	for (i = 0; i < size; i++) {
		sprintf(hexstr + i * 2, "%02X", data[i]);
	}
	hexstr[i * 2] = 0;
	return hexstr;
}

extern "C" LEXIAJ2534_API long PassThruOpen(void* pName, unsigned long* pDeviceID)
{
	AllocConsole();
	FILE* f = freopen("CONIN$", "r", stdin);
	f = freopen("CONOUT$", "w", stdout);
	f = freopen("CONOUT$", "w", stderr);
	cout << endl << "PassThruOpen" << endl << endl;
	return translator->OpenDevice(pDeviceID);
}

extern "C" LEXIAJ2534_API long PassThruReadVersion(
	unsigned long DeviceID,
	char* pFirmwareVersion,
	char* pDllVersion,
	char* pApiVersion)
{
	if (translator->GetFmwVer(DeviceID, pFirmwareVersion) == STATUS_NOERROR)
	{
		strcpy(pDllVersion, "1.00");
		strcpy(pApiVersion, "04.04");
		return STATUS_NOERROR;
	}
	return ERR_FAILED;
}

extern "C" LEXIAJ2534_API long PassThruClose(unsigned long DeviceID)
{
#ifdef LOG_OUTPUT
	cout << endl << "Close" << endl;
	cout << "DeviceID: " << DeviceID << endl;
#endif 		
	return translator->CloseDevice(DeviceID);
}
extern "C" LEXIAJ2534_API long PassThruConnect(
	unsigned long DeviceID,
	unsigned long ProtocolID,
	unsigned long Flags,
	unsigned long BaudRate,
	unsigned long* pChannelID)
{
#ifdef LOG_OUTPUT
	cout << endl << "Connect" << endl;
	cout << "DeviceID: " << dec << DeviceID << endl;
	cout << "ProtocolID: " << dec << ProtocolID << endl;
	cout << "Flags: 0x" << hex << Flags << endl;
	cout << "BaudRate: " << dec << BaudRate << endl;
#endif 
	if (ProtocolID != ISO15765 || BaudRate != 500000 || Flags != 0)
	{
		return ERR_NOT_SUPPORTED;
	}
	return translator->ConnectAsCan500(DeviceID, pChannelID);
}
extern "C" LEXIAJ2534_API long PassThruDisconnect(
	unsigned long ChannelID)
{
#ifdef LOG_OUTPUT
	cout << endl << "Disconnect" << endl;
#endif 
	return translator->Disconnect(ChannelID);
}
extern "C" LEXIAJ2534_API long PassThruReadMsgs(
	unsigned long ChannelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pNumMsgs,
	unsigned long Timeout)
{
#ifdef LOG_OUTPUT
	cout << endl << "Read" << endl;
	cout << "ChannelID: " << ChannelID << endl;
	cout << "Timeout: " << Timeout << endl;
	cout << "pNumMsgs: " << *pNumMsgs << endl;
#endif 	
	size_t len = 0;
	unsigned short addr = 0;
	long status = translator->ReadMsg(ChannelID, &pMsg->Data[4], &len, &addr);
	if (status == STATUS_NOERROR)
	{
		memset(pMsg->Data, 0x00, 4);
		pMsg->Data[2] = (addr >> 8);
		pMsg->Data[3] = (addr & 0xFF);
		pMsg->ProtocolID = ISO15765;
		pMsg->RxStatus = 0;
		pMsg->DataSize = len + 4;
	}
	return status;
}
extern "C" LEXIAJ2534_API long PassThruWriteMsgs(
	unsigned long ChannelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pNumMsgs,
	unsigned long Timeout)
{
#ifdef LOG_OUTPUT
	cout << endl << "W" << endl;
	cout << dec << clock() - clk << endl;
	clk = clock();
	cout << "ChannelID: " << ChannelID << endl;
	cout << "ProtocolID: " << pMsg->ProtocolID << endl;
	cout << "RxStatus: " << pMsg->RxStatus << endl;
	cout << "TxFlags: 0x" << hex << pMsg->TxFlags << endl;
	cout << "pNumMsgs: " << dec << *pNumMsgs << endl;
	cout << "Timeout: " << Timeout << endl;
	cout << "Data: " << BytesToCharArray(pMsg->Data, pMsg->DataSize) << endl;
#endif 	
	if (pMsg->DataSize <= 4 || *pNumMsgs > 1 || pMsg->ProtocolID != ISO15765)
	{
		return ERR_NOT_SUPPORTED;
	}
	unsigned short ecu = pMsg->Data[2] << 8;
	ecu |= pMsg->Data[3];

	return translator->SendMsg(ChannelID, &pMsg->Data[4], pMsg->DataSize - 4, ecu);
}
extern "C" LEXIAJ2534_API long PassThruStartPeriodicMsg(
	unsigned long ChannelID,
	PASSTHRU_MSG* pMsg,
	unsigned long* pMsgID,
	unsigned long TimeInterval)
{
	cout << endl << "PeriodicMsg " << endl;
	cout << "ChannelID: " << ChannelID << endl;
	cout << "ProtocolID: " << pMsg->ProtocolID << endl;
	cout << "RxStatus: " << pMsg->RxStatus << endl;
	cout << "TxFlags: " << pMsg->TxFlags << endl;
	cout << "TimeInterval: " << TimeInterval << endl;
	cout << "Data: " << BytesToCharArray(pMsg->Data, pMsg->DataSize) << endl;
	return ERR_FAILED;;
}
extern "C" LEXIAJ2534_API long PassThruStopPeriodicMsg(
	unsigned long ChannelID,
	unsigned long MsgID)
{
	cout << endl << "PassThruStopPeriodicMsg " << endl;
	cout << "ChannelID: " << ChannelID << endl;
	cout << "MsgID: " << MsgID << endl;
	return ERR_FAILED;;
}
extern "C" LEXIAJ2534_API long PassThruStartMsgFilter(
	unsigned long ChannelID,
	unsigned long FilterType,
	PASSTHRU_MSG* pMaskMsg,
	PASSTHRU_MSG* pPatternMsg,
	PASSTHRU_MSG* pFlowControlMsg,
	unsigned long* pFilterID)
{
	if (pMaskMsg == NULL || pPatternMsg == NULL || pFlowControlMsg == NULL)
	{
		return ERR_NULL_PARAMETER;
	}
	if (FilterType != FLOW_CONTROL_FILTER)
	{
		return ERR_NOT_SUPPORTED;
	}
	if (pMaskMsg->ProtocolID != ISO15765 || pPatternMsg->ProtocolID != ISO15765 || pFlowControlMsg->ProtocolID != ISO15765)
	{
		return ERR_INVALID_PROTOCOL_ID;
	}
	if (pMaskMsg->TxFlags != ISO15765_FRAME_PAD || pPatternMsg->TxFlags != ISO15765_FRAME_PAD || pFlowControlMsg->TxFlags != ISO15765_FRAME_PAD)
	{
		return ERR_INVALID_FLAGS;
	}
	if (pPatternMsg->DataSize < 4 || pFlowControlMsg->DataSize < 4)
	{
		return ERR_INVALID_MSG;
	}

	cout << endl << "MsgFilter " << endl;

	cout << "---------" << endl << "pMaskMsg" << endl << "---------" << endl;
	cout << "ProtocolID: " << dec << pMaskMsg->ProtocolID << endl;
	cout << "TxFlags: 0x" << hex << pMaskMsg->TxFlags << endl;
	cout << "Data: " << BytesToCharArray(pMaskMsg->Data, pMaskMsg->DataSize) << endl;

	cout << "---------" << endl << "pPatternMsg" << endl << "---------" << endl;
	cout << "ProtocolID: " << dec << pPatternMsg->ProtocolID << endl;
	cout << "TxFlags: 0x" << hex << pPatternMsg->TxFlags << endl;
	cout << "Data: " << BytesToCharArray(pPatternMsg->Data, pPatternMsg->DataSize) << endl;

	cout << "---------" << endl << "pFlowControlMsg" << endl << "---------" << endl;
	cout << "ProtocolID: " << dec << pFlowControlMsg->ProtocolID << endl;
	cout << "TxFlags: 0x" << hex << pFlowControlMsg->TxFlags << endl;
	cout << "Data: " << BytesToCharArray(pFlowControlMsg->Data, pFlowControlMsg->DataSize) << endl;

	unsigned short ecu = pFlowControlMsg->Data[2] << 8;
	ecu |= pFlowControlMsg->Data[3];
	unsigned short ans = pPatternMsg->Data[2] << 8;
	ans |= pPatternMsg->Data[3];

	return translator->SetFilter(ChannelID, ecu, ans, pFilterID);
}
extern "C" LEXIAJ2534_API long PassThruStopMsgFilter(
	unsigned long ChannelID,
	unsigned long FilterID)
{
	cout << endl << "PassThruStopMsgFilter " << endl;
	cout << "ChannelID: " << ChannelID << endl;
	cout << "FilterID: " << FilterID << endl;
	return ERR_FAILED;
}

extern "C" LEXIAJ2534_API long PassThruSetProgrammingVoltage(
	unsigned long DeviceID,
	unsigned long PinNumber,
	unsigned long Voltage)
{
	cout << endl << "PassThruSetProgrammingVoltage " << endl;
	cout << dec << clock() - clk << endl;
	clk = clock();
	cout << "DeviceID: " << dec << DeviceID << endl;
	cout << "PinNumber: " << dec << PinNumber << endl;
	cout << "Voltage: 0x" << hex << Voltage << endl;
	return ERR_FAILED;
}

extern "C" LEXIAJ2534_API long PassThruGetLastError(
	char* pErrorDescription)
{
	cout << endl << "PassThruGetLastError " << endl; 
	memcpy(pErrorDescription, errorStr, sizeof(errorStr));
	return STATUS_NOERROR;
}
extern "C" LEXIAJ2534_API long PassThruIoctl(
	unsigned long ChannelID,
	unsigned long IoctlID,
	void* pInput,
	void* pOutput)
{
	cout << endl << "Ioctl" << endl;
	cout << dec << clock() - clk << endl;
	clk = clock();
	cout << "ChannelID: " << ChannelID << endl;
	cout << "IoctlID: " << IoctlID << endl;
	int num_pars;
	ULONG bat;
	switch (IoctlID)
	{
	case SET_CONFIG:
		cout << "SET_CONFIG" << endl;
		num_pars = ((SCONFIG_LIST*)pInput)->NumOfParams;
		cout << "NumOfParams: " << num_pars << endl;
		for (int i = 0; i < num_pars; i++)
		{
			cout << "Num: " << i << endl;
			cout << "Parameter: " << (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Parameter << endl;
			cout << "Value: " << (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Value << endl;
		}
		return ERR_NOT_SUPPORTED;
		break;
	case READ_VBATT:
		bat = 14400;
		cout << "READ_VBATT" << endl;
		cout << bat << endl;
		memcpy(pOutput, &bat, sizeof(bat));
		return STATUS_NOERROR;
		break;
	case CLEAR_RX_BUFFER:
	case CLEAR_TX_BUFFER:
		cout << "Clear buffer" << endl;
		return translator->ClearBuffers(ChannelID);
	case FIVE_BAUD_INIT:
	{
		return ERR_NOT_SUPPORTED;
	}
	case FAST_INIT:
	{
		cout << "FAST_INIT" << endl; return ERR_FAILED;
	}
	case CLEAR_PERIODIC_MSGS:
	{
		return ERR_NOT_SUPPORTED;
	}
	case CLEAR_MSG_FILTERS:
		cout << "CLEAR_MSG_FILTERS" << endl; 
		return translator->ClearFilters(ChannelID);
	default:
		return ERR_NOT_SUPPORTED;;
	}
	return STATUS_NOERROR;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

