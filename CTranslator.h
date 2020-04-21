#pragma once
#include "CLexiaExchanger.h"
#define NOT_INITIALIZED 0xFFFFFFFF
class CTranslator
{
public:
	CTranslator();
	long OpenDevice(unsigned long* id);
	long CloseDevice(unsigned long id);
	long GetFmwVer(unsigned long id, char* fmw_str);

	long ConnectAsCan500(unsigned long id, unsigned long* ch);
	long Disconnect(unsigned long ch);

	long SendMsg(unsigned long ch, unsigned char* buf, size_t len, unsigned short addr);
	long ReadMsg(unsigned long ch, unsigned char* buf, size_t* len, unsigned short* addr);

	long SetFilter(unsigned long ch, unsigned short ecu_addr, unsigned short ans_addr, unsigned long* id);
	long StopFilter(unsigned long ch, unsigned long id);
	long ClearFilters(unsigned long ch);
	long ClearBuffers(unsigned long ch);

	~CTranslator();
private:
	CLexiaExchanger* Lexia;
	unsigned long device;
	unsigned long channel;
	unsigned long filter;
	unsigned long sendLen;
	char tempBuf[1024];
	char sendBuf[1024];
};

