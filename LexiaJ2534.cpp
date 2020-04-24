#include <Windows.h>
#include "j2534.h"
#include "CTranslator.h"
#include <ctime>
#include <consoleapi3.h>
#include <iostream>
#include "dllmain.h"

#define LOG_OUTPUT
char errorStr[] = "TODO: ERRORS DESCRIPTIONS";
using namespace std;

#ifdef LEXIAJ2534_EXPORTS
#define LEXIAJ2534_API __declspec(dllexport)
#else
#define LEXIAJ2534_API __declspec(dllimport)
#endif

CTranslator* translator = new CTranslator();

extern "C" LEXIAJ2534_API long APIENTRY PassThruOpen(void* pName, unsigned long* pDeviceID)
{
	LexiaLog("PassThruOpen");
	return translator->OpenDevice(pDeviceID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruReadVersion(unsigned long DeviceID, char* pFirmwareVersion, char* pDllVersion, char* pApiVersion)
{
	strcpy(pDllVersion, "1.00");
	strcpy(pApiVersion, "04.04");
	return translator->GetFmwVer(DeviceID, pFirmwareVersion);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruClose(unsigned long DeviceID)
{
	LexiaLog("PassThruClose");
	LexiaLog("DeviceID: %d", DeviceID);
	return translator->CloseDevice(DeviceID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruConnect(unsigned long DeviceID, unsigned long ProtocolID, unsigned long Flags, unsigned long BaudRate, unsigned long* pChannelID)
{
	LexiaLog("PassThruConnect: DeviceID - %d, ProtocolID - %d, Flags: 0x%08X, Baudrate - %d", DeviceID, ProtocolID, Flags, BaudRate);
	if (ProtocolID != ISO15765 || BaudRate != 500000 || Flags != 0)
	{
		return ERR_NOT_SUPPORTED;
	}
	return translator->ConnectAsCan500(DeviceID, pChannelID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruDisconnect(unsigned long ChannelID)
{
	LexiaLog("PassThruDisconnect");
	return translator->Disconnect(ChannelID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruReadMsgs(unsigned long ChannelID, PASSTHRU_MSG* pMsg, unsigned long* pNumMsgs, unsigned long Timeout)
{
	if (pMsg == NULL || pNumMsgs == NULL)
	{
		return ERR_NULL_PARAMETER;
	}
	LexiaLog("PassThruReadMsgs: ChannelID - %d, Timeout - %d, pNumMsgs - %d", ChannelID, Timeout, *pNumMsgs);
	size_t len = 0;
	unsigned short addr = 0;
	//Reading message
	long status = translator->ReadMsg(ChannelID, &pMsg->Data[4], &len, &addr);
	if (status == STATUS_NOERROR)
	{
		//Filling answer address and other stuff
		memset(pMsg->Data, 0x00, 4);
		pMsg->Data[2] = (addr >> 8);
		pMsg->Data[3] = (addr & 0xFF);
		pMsg->ProtocolID = ISO15765;
		pMsg->RxStatus = 0;
		pMsg->DataSize = len + 4;
		*pNumMsgs = 1;
	}
	return status;
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruWriteMsgs(unsigned long ChannelID, PASSTHRU_MSG* pMsg, unsigned long* pNumMsgs, unsigned long Timeout)
{
	if (pMsg == NULL || pNumMsgs == NULL)
	{
		return ERR_NULL_PARAMETER;
	}

	LexiaLog("PassThruWriteMsgs: ChannelID - %d, Timeout - %d, pNumMsgs - %d, TxFlags - %08X, Data - %s, DataSize - %d",
		ChannelID, Timeout, *pNumMsgs, pMsg->TxFlags, BytesToCharArray(pMsg->Data, pMsg->DataSize), pMsg->DataSize);

	if (pMsg->DataSize < 4 || *pNumMsgs > 1 || pMsg->ProtocolID != ISO15765)
	{
		return ERR_NOT_SUPPORTED;
	}
	//Take ECU address from message
	//0000XXXX000000~ - XXXX - address in PassThruMsg
	unsigned short ecu = (pMsg->Data[2] << 8);
	ecu |= pMsg->Data[3];
	return translator->SendMsg(ChannelID, &pMsg->Data[4], pMsg->DataSize - 4, ecu);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruStartPeriodicMsg(unsigned long ChannelID, PASSTHRU_MSG* pMsg, unsigned long* pMsgID, unsigned long TimeInterval)
{
	LexiaLog("PassThruStartPeriodicMsg: ChannelID - %d, TimeInterval - %d, TxFlags - %08X, Data - %s", ChannelID, TimeInterval, pMsg->TxFlags, BytesToCharArray(pMsg->Data, pMsg->DataSize));
	return STATUS_NOERROR;
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruStopPeriodicMsg(unsigned long ChannelID, unsigned long MsgID)
{
	LexiaLog("PassThruStopPeriodicMsg: ChannelID - %d, MsgID - %d", ChannelID, MsgID);
	return STATUS_NOERROR;
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruStartMsgFilter(unsigned long ChannelID, unsigned long FilterType, PASSTHRU_MSG* pMaskMsg, PASSTHRU_MSG* pPatternMsg, PASSTHRU_MSG* pFlowControlMsg, unsigned long* pFilterID)
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

	LexiaLog("PassThruStartMsgFilter: ChannelID - %d, FilterType - %d", ChannelID, FilterType);
	LexiaLog("MaskMsg:        %s", BytesToCharArray(pMaskMsg->Data, pMaskMsg->DataSize));
	LexiaLog("PatternMsg:     %s", BytesToCharArray(pPatternMsg->Data, pPatternMsg->DataSize));
	LexiaLog("FlowControlMsg: %s", BytesToCharArray(pFlowControlMsg->Data, pFlowControlMsg->DataSize));

	//Take addresses from structs
	unsigned short ecu = (pFlowControlMsg->Data[2] << 8);
	ecu |= pFlowControlMsg->Data[3];
	unsigned short ans = (pPatternMsg->Data[2] << 8);
	ans |= pPatternMsg->Data[3];

	return translator->SetFilter(ChannelID, ecu, ans, pFilterID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruStopMsgFilter(unsigned long ChannelID, unsigned long FilterID)
{
	LexiaLog("PassThruStopMsgFilter: ChannelID - %d, FilterID - %d", ChannelID, FilterID);
	return translator->StopFilter(ChannelID, FilterID);
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruSetProgrammingVoltage(unsigned long DeviceID, unsigned long PinNumber, unsigned long Voltage)
{
	LexiaLog("PassThruSetProgrammingVoltage: DeviceID - %d, PinNumber - %d, Voltage - %d", DeviceID, PinNumber, Voltage);
	return ERR_NOT_SUPPORTED;
}

extern "C" LEXIAJ2534_API long APIENTRY PassThruGetLastError(char* pErrorDescription)
{
	LexiaLog("PassThruGetLastError");
	strcpy(pErrorDescription, errorStr);
	return STATUS_NOERROR;
}
extern "C" LEXIAJ2534_API long APIENTRY PassThruIoctl(unsigned long ChannelID, unsigned long IoctlID, void* pInput, void* pOutput)
{
	LexiaLog("PassThruIoctl: ChannelID - %d, IoctlID - %d", ChannelID, IoctlID);
	int num_pars = 0;
	ULONG bat = 14400;
	switch (IoctlID)
	{
	case SET_CONFIG:
	{
		LexiaLog("SET_CONFIG");
		if (pInput == NULL)
		{
			return ERR_NULL_PARAMETER;
		}
		num_pars = ((SCONFIG_LIST*)pInput)->NumOfParams;
		for (int i = 0; i < num_pars; i++)
		{
			LexiaLog("Parameter: %d, Value: %d", (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Parameter, (SCONFIG*)(((SCONFIG_LIST*)pInput)->ConfigPtr)[i].Value);
		}
		return ERR_NOT_SUPPORTED;
		break;
	}
	case READ_VBATT:
	{
		*(unsigned long*)pOutput = bat;
		return STATUS_NOERROR;
	}
	case CLEAR_RX_BUFFER:
	case CLEAR_TX_BUFFER:
	{
		LexiaLog("Clear buffers");
		return translator->ClearBuffers(ChannelID);
	}
	case CLEAR_MSG_FILTERS:
	{
		LexiaLog("CLEAR_MSG_FILTERS");
		return translator->ClearFilters(ChannelID);
	}
	case CLEAR_PERIODIC_MSGS:
		return STATUS_NOERROR;
	case FIVE_BAUD_INIT:
	case FAST_INIT:
	
	default:
	{
		return ERR_NOT_SUPPORTED;
	}
	}
	return STATUS_NOERROR;
}