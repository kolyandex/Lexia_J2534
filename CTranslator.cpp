#include "pch.h"
#include "CTranslator.h"
#include "j2534.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

unsigned char AppVersion[] = { 0x00, 0xFA, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char EepromRead[] = { 0x00, 0x09, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,0x00, 0xFF };

static unsigned char protocolMessage[] = {
	0x00, 0x16, 0xAA, 0x04,
	0x7C, 0x15, 0x00, 0x00,
	0x76, 0x00, 0x00, 0x00,
	0x54, 0x00, 0x64, 0x01, 0x54, 0x01, 0x0A, 0x00,
	0x54, 0x02, 0x0D, 0x00, 0x54, 0x03, 0x64, 0x01,
	0x54, 0x04, 0x0A, 0x00, 0x54, 0x05, 0x32, 0x00,
	0x54, 0x09, 0x00, 0x00, 0x54, 0x0A, 0xC4, 0x09,
	0x54, 0x0C, 0xF4, 0x01, 0x50, 0x00, 0x08, 0x00,

	0x50, 0x01, 0x84, 0x07,
	0x50, 0x02, 0x85, 0x07,

	0x50, 0x03, 0x00, 0x00,
	0x43, 0x55,
	0x00, 0x00, 0x01, 0x01, 0x56, 0x00, 0xFF, 0x00,
	0x52, 0x00, 0xFF, 0x00, 0x43, 0x55, 0x00, 0x00,
	0x02, 0x02, 0x56, 0x00, 0xFF, 0x00, 0x52, 0x00,
	0xFF, 0x00, 0x43, 0x50,
	0xD0, 0x07, //periodic time
	0x03, 0x00,	0x44, 0x3E, 0x44, 0x02, // periodoc message
	0x43, 0x55, 0x00, 0x00,
	0x04, 0x00, 0x56, 0x00, 0xFF, 0x00, 0x52, 0x00,
	0xFF, 0x00, 0x43, 0x55, 0x00, 0x00, 0xFF, 0x00,
	0x56, 0x00, 0xFF, 0x00, 0x52, 0x00, 0xFF, 0x00 };

unsigned char someMsg[] = { 0x00, 0x09, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,0x16, 0x06 };
unsigned char someVerReq[] = { 0x00, 0x09, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,0x00, 0x1C };

unsigned char CommStatus[] = { 0x00, 0xFE, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char someMsg4[] = { 0x00, 0x0B, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04 };
unsigned char someMsg5[] = { 0x00, 0x0B, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03 };
unsigned char JumpToApp[] = { 0x00, 0x12, 0x00, 0x00, 0x7C, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
unsigned char connectAsCan[] = { 0x00, 0x05, 0xAA, 0x02, 0x7C, 0x15, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x46, 0x46, 0x58, 0x58, 0x58, 0x58, 0x43, 0x30, 0x32, 0x2B, 0x32, 0x58, 0x58, 0x43, 0x35, 0x58, 0x58, 0x58, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 };

unsigned char sendMessage[] = { 0xFF, 0x01, 0x00, 0x04, 0x74, 0x40, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1A, 0x87 };
unsigned char sendMessage2[] = { 0xFF, 0x01, 0x00, 0x04, 0x74, 0x40, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x92 };
unsigned char sendMessage3[] = { 0xFF, 0x01, 0x00, 0x04, 0x74, 0x40, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x85 };
static void Replace(char* start, char* end, char old_val, char new_val)
{
	if (start > end)
		return;
	while (start != end)
	{
		if (*start == old_val)
		{
			*start = new_val;
		}
		start++;
	}
}

CTranslator::CTranslator()
{
	Lexia = new CLexiaExchanger();
	device = NOT_INITIALIZED;
	channel = NOT_INITIALIZED;
	filter = NOT_INITIALIZED;
	sendLen = 0;
	memset(tempBuf, 0x00, sizeof(tempBuf));
}

long CTranslator::OpenDevice(unsigned long* id)
{
	if (device != NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}

	LEXIA_STATUS status = Lexia->Connect();

	if (status == LEXIA_NOERROR)
	{
		std::srand(static_cast<unsigned int>(std::time(0)));
		device = std::rand();
		*id = device;
		return STATUS_NOERROR;
	}
	else
	{
		printf("Open device: %s\n", LexiaErrors[status]);
		return ERR_FAILED;
	}
}
long CTranslator::CloseDevice(unsigned long id)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_FAILED;
	}
	if (id != device)
	{
		return ERR_INVALID_DEVICE_ID;
	}
	if (Lexia->Disconnect() == LEXIA_NOERROR)
	{
		device = NOT_INITIALIZED;
		return STATUS_NOERROR;
	}
	return ERR_FAILED;
}
long CTranslator::GetFmwVer(unsigned long id, char* fmw_str)
{
	if (id != device)
	{
		return ERR_FAILED;
	}
	size_t received_len = 0;
	if (Lexia->SendReceive(AppVersion, sizeof(AppVersion), tempBuf, &received_len) == LEXIA_NOERROR)
	{
		Replace(&tempBuf[20], &tempBuf[59], 0x00, ' ');
		printf("Firmware: %s\n", &tempBuf[20]);
		strcpy(fmw_str, &tempBuf[20]);
		return STATUS_NOERROR;
	}
	return ERR_FAILED;
}

long CTranslator::ConnectAsCan500(unsigned long id, unsigned long* ch)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (id != device)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (channel != NOT_INITIALIZED)
	{
		return ERR_FAILED;
	}
	size_t received_len = 0;

	//Lexia->SendReceive(someMsg, sizeof(someMsg), tempBuf, &received_len);
	//Lexia->SendReceive(someMsg3, sizeof(someMsg3), tempBuf, &received_len);
	//Lexia->SendReceive(someMsg2, sizeof(someMsg2), tempBuf, &received_len);
	//Lexia->SendReceive(someMsg3, sizeof(someMsg3), tempBuf, &received_len);
	//Lexia->SendReceive(someVerReq, sizeof(someVerReq), tempBuf, &received_len);
	Lexia->SendReceive(JumpToApp, sizeof(JumpToApp), tempBuf, &received_len);//!
	Lexia->SendReceive(CommStatus, sizeof(CommStatus), tempBuf, &received_len);//!
	//Lexia->SendReceive(EepromRead, sizeof(EepromRead), tempBuf, &received_len);//!
	//Lexia->SendReceive(someMsg4, sizeof(someMsg4), tempBuf, &received_len);
	//Lexia->SendReceive(someMsg5, sizeof(someMsg5), tempBuf, &received_len);

	while (true)
	{
		if (Lexia->SendReceive(connectAsCan, sizeof(connectAsCan), tempBuf, &received_len) == LEXIA_NOERROR)
		{
			if (tempBuf[14] == 0x29) continue;
			else break;
		}
		else
			return ERR_FAILED;
	}

	//if (Lexia->SendReceive(connectAsCan, sizeof(connectAsCan), tempBuf, &received_len) == LEXIA_NOERROR)
	{
		//Sleep(1000);
		//Lexia->SendReceive(connectAsCan, sizeof(connectAsCan), tempBuf, &received_len);
		channel = std::rand();
		*ch = channel;
		//Lexia->SendReceive(protocolMessage, sizeof(protocolMessage), tempBuf, &received_len);
		//Lexia->SendReceive(sendMessage, sizeof(sendMessage), tempBuf, &received_len);
		//Lexia->SendReceive(sendMessage2, sizeof(sendMessage2), tempBuf, &received_len);
		//Lexia->SendReceive(sendMessage3, sizeof(sendMessage3), tempBuf, &received_len);
		return STATUS_NOERROR;
	}
	return ERR_FAILED;
}
long CTranslator::Disconnect(unsigned long ch)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	channel = NOT_INITIALIZED;
	return STATUS_NOERROR;
}

long CTranslator::SendMsg(unsigned long ch, unsigned char* buf, size_t len, unsigned short addr)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	if (filter == NOT_INITIALIZED)
	{
		return ERR_NO_FLOW_CONTROL;
	}
	if (len == 0)
	{
		return ERR_BUFFER_EMPTY;
	}
	if (addr != *(unsigned short*)& protocolMessage[54])
	{
		printf("ECU addr not match!\n");
		return ERR_NOT_SUPPORTED;
	}	
	if (sendLen != 0)
	{
		return ERR_BUFFER_FULL;
	}
	memcpy(sendBuf, buf, len);
	sendLen = len;
	return STATUS_NOERROR;
}
long CTranslator::ReadMsg(unsigned long ch, unsigned char* buf, size_t* len, unsigned short* addr)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	if (filter == NOT_INITIALIZED)
	{
		return ERR_NO_FLOW_CONTROL;
	}
	if (sendLen == 0)
	{
		return ERR_BUFFER_EMPTY;
	}
	size_t received_len = 0;
	Lexia->SendReceive(protocolMessage, sizeof(protocolMessage), tempBuf, &received_len);
	unsigned long total_len = 12 + sendLen;
	unsigned char* tmpSendBuf = new unsigned char[total_len];
	memset(tmpSendBuf, 0x00, total_len);
	tmpSendBuf[0] = 0xFF;
	tmpSendBuf[1] = 0x01;

	unsigned long timeout = 8250;
	*((unsigned long*)tmpSendBuf + 1) = timeout;
	*((unsigned long*)tmpSendBuf + 2) = sendLen;
	memcpy(&tmpSendBuf[12], sendBuf, sendLen);
	sendLen = 0;
	//001200000000000000000012 00 000CAA00000000
	//FF010200000000000000FF01 07 0001AA0000000001FF0200055092
	received_len = 0;
	Lexia->SendReceive(tmpSendBuf, total_len, tempBuf, &received_len);

	if (received_len >= 20)
	{
		if (tempBuf[14] != 0x01)
		{			
			return ERR_TIMEOUT;
		}
		if (tempBuf[12] >= 3)
		{
			if (tempBuf[22] > 0)
			{
				memcpy(buf, &tempBuf[25], tempBuf[22]);
				*len = tempBuf[22];
				*addr = *(unsigned short*)& protocolMessage[58];
				return STATUS_NOERROR;
			}			
		}
	}
	return ERR_FAILED;
}

long CTranslator::SetFilter(unsigned long ch, unsigned short ecu_addr, unsigned short ans_addr, unsigned long* id)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	if (filter != NOT_INITIALIZED)
	{
		return ERR_EXCEEDED_LIMIT;
	}
	filter = rand();
	*id = filter;
	*(unsigned short*)& protocolMessage[54] = ecu_addr;
	*(unsigned short*)& protocolMessage[58] = ans_addr;
	printf("ECU: %X\n", *(unsigned short*)& protocolMessage[54]);
	printf("ANS: %X\n", *(unsigned short*)& protocolMessage[58]);
	return STATUS_NOERROR;
}
long CTranslator::ClearFilters(unsigned long ch)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	filter = NOT_INITIALIZED;
	return STATUS_NOERROR;
}
long CTranslator::ClearBuffers(unsigned long ch)
{
	if (device == NOT_INITIALIZED)
	{
		return ERR_DEVICE_NOT_CONNECTED;
	}
	if (ch != channel || channel == NOT_INITIALIZED)
	{
		return ERR_INVALID_CHANNEL_ID;
	}
	sendLen = 0;
	return STATUS_NOERROR;
}


CTranslator::~CTranslator()
{

}