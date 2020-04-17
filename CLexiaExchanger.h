#pragma once
#define LEXIA_NOERROR 0
#define LEXIA_NOT_CONNECTED 1
#define LEXIA_IO_ERROR 2
#define LEXIA_FAILED 3
#define LEXIA_IN_USE 4

typedef int LEXIA_STATUS;
static const char* LexiaErrors[] =
{
	"LEXIA_NOERROR",
	"LEXIA_NOT_CONNECTED",
	"LEXIA_IO_ERROR",
	"LEXIA_FAILED",
	"LEXIA_IN_USE"
};

class CLexiaExchanger
{
public:
	ULONG LexiaId;

	CLexiaExchanger();
	LEXIA_STATUS Connect();
	LEXIA_STATUS Disconnect();
	LEXIA_STATUS SendReceive(void* in, size_t in_len, void* out, size_t* out_len);
	~CLexiaExchanger();

private:
	HANDLE Device;
	LEXIA_STATUS GetDevicePath(_In_  LPGUID InterfaceGuid, _Out_writes_z_(BufLen) PWCHAR DevicePath, _In_ size_t BufLen);
};

