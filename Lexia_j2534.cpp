#include "pch.h"
#include "j2534.h"
#include "CTranslator.h"
#include <ctime>
#include <consoleapi3.h>
#include <iostream>

#define LOG_OUTPUT
using namespace std;

#ifdef LEXIAJ2534_EXPORTS
#define LEXIAJ2534_API __declspec(dllexport)
#else
#define LEXIAJ2534_API __declspec(dllimport)
#endif

clock_t clk = clock();
CTranslator* translator = new CTranslator();


char receiveBuf[2048];
static char* BytesToCharArray(UCHAR* data, UINT size)
{
	char hexstr[8192];
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
	return ERR_FAILED;;
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
#ifdef LOG_OUTPUT
	cout << "Data: " << BytesToCharArray(pMsg->Data, pMsg->DataSize) << endl;
#endif 
	return ERR_FAILED;
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
	return ERR_FAILED;;
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
	cout << endl << "MsgFilter " << endl;

	cout << "ChannelID: " << ChannelID << endl;
	cout << "FilterType: " << FilterType << endl;

	cout << "---------" << endl << "pMaskMsg" << endl << "---------" << endl;
	cout << "ProtocolID: " << dec << pMaskMsg->ProtocolID << endl;
	cout << "TxFlags: 0x" << hex << pMaskMsg->TxFlags << endl;
	cout << "Data: " << BytesToCharArray(pMaskMsg->Data, pMaskMsg->DataSize) << endl;

	cout << "---------" << endl << "pPatternMsg" << endl << "---------" << endl;
	cout << "ProtocolID: " << dec << pPatternMsg->ProtocolID << endl;
	cout << "TxFlags: 0x" << hex << pPatternMsg->TxFlags << endl;
	cout << "Data: " << BytesToCharArray(pPatternMsg->Data, pPatternMsg->DataSize) << endl;

	if (pFlowControlMsg != nullptr)
	{
		cout << "---------" << endl << "pFlowControlMsg" << endl << "---------" << endl;
		cout << "ProtocolID: " << dec << pFlowControlMsg->ProtocolID << endl;
		cout << "TxFlags: 0x" << hex << pFlowControlMsg->TxFlags << endl;
		cout << "Data: " << BytesToCharArray(pFlowControlMsg->Data, pFlowControlMsg->DataSize) << endl;
	}
	else
	{
		cout << "pFlowControlMsg is NULL" << endl;
	}
	return ERR_FAILED;;
}
extern "C" LEXIAJ2534_API long PassThruStopMsgFilter(
	unsigned long ChannelID,
	unsigned long FilterID)
{
	cout << endl << "PassThruStopMsgFilter " << endl;
	cout << "ChannelID: " << ChannelID << endl;
	cout << "FilterID: " << FilterID << endl;
	return ERR_FAILED;;
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
	return ERR_FAILED;;;
}

extern "C" LEXIAJ2534_API long PassThruGetLastError(
	char* pErrorDescription)
{
	cout << endl << "PassThruGetLastError " << endl; return ERR_FAILED;;
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
			/*if (((SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr))[i].Parameter > 0x1F)
			{
				((SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr))[i].Parameter = 0x02;
				((SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr))[i].Value = 0x00;
			}*/
			cout << "Num: " << i << endl;
			cout << "Parameter: " << (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Parameter << endl;
			cout << "Value: " << (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Value << endl;
		}return ERR_FAILED;;
		break;
	case READ_VBATT:
		bat = 12300;
		cout << "READ_VBATT" << endl;
		cout << bat << endl;
		memcpy(pOutput, &bat, sizeof(bat));
		break;
	case CLEAR_RX_BUFFER:
	case CLEAR_TX_BUFFER:
		cout << "Clear buffer" << endl;
		return ERR_FAILED;;
	case FIVE_BAUD_INIT:
	{
		cout << "FIVE_BAUD_INIT" << endl;
		SBYTE_ARRAY* inputMsg = (SBYTE_ARRAY*)pInput;
		cout << "InputMsg data: " << BytesToCharArray(inputMsg->BytePtr, inputMsg->NumOfBytes) << endl;
		SBYTE_ARRAY* outputMsg = (SBYTE_ARRAY*)pOutput;
		cout << "OutputMsg data: " << BytesToCharArray(outputMsg->BytePtr, outputMsg->NumOfBytes) << endl;
		return ERR_FAILED;
	}
	case FAST_INIT:
	{
		cout << "FAST_INIT" << endl; return ERR_FAILED;
	}
	case CLEAR_PERIODIC_MSGS:
	case CLEAR_MSG_FILTERS:
		cout << "CLEAR_PERIODIC_MSGS CLEAR_MSG_FILTERS" << endl; return ERR_FAILED;;
	default:
		cout << "Not implemented ID: " << IoctlID << endl; return ERR_FAILED;;
	}
	return STATUS_NOERROR;
}

BOOL APIENTRY DllMain( HMODULE hModule,
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

