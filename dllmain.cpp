#include "framework.h"
#include <iostream>
#include <ctime>
FILE* outFile = NULL;
static clock_t clk = clock();
static char printf_buf[1024];
static char hexstr[1024];

char* BytesToCharArray(UCHAR* data, UINT size)
{
	UINT i = 0;
	if (size < (sizeof(hexstr) / sizeof(hexstr[0])) / 2)
	{
		for (; i < size; i++) 
		{
			sprintf(hexstr + i * 2, "%02X", data[i]);
		}
	}
	hexstr[i * 2] = 0;
	return hexstr;
}

void LexiaLog(char* format, ...)
{
	int s_len = 0;
	va_list args;
	va_start(args, format);
	s_len = vsnprintf(NULL, 0, format, args);

	if ((s_len + 10) > (sizeof(printf_buf) / sizeof(printf_buf[0])))
	{
		printf("printf_buf too small\n");
		return;
	}

	s_len = sprintf(printf_buf, "%06d - ", clock());
	if (s_len > 0)
	{
		s_len = vsnprintf((char*)& printf_buf[s_len], (sizeof(printf_buf) / sizeof(printf_buf[0])) - s_len, format, args);
	}	
	va_end(args);

	if (s_len < 0)
	{
		return;
	}
	printf("%s\n", printf_buf);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (AllocConsole())
		{
			outFile = freopen("CONOUT$", "w", stdout);
		}
		memset(printf_buf, 0x00, sizeof(printf_buf) / sizeof(printf_buf[0]));
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		if (outFile != NULL) fclose(outFile);
		break;
	}
	}
	return TRUE;
}

