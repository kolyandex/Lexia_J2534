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
	if (outFile == NULL) outFile = freopen("CONOUT$", "w", stdout);
	if ((s_len + 10) <= (sizeof(printf_buf) / sizeof(printf_buf[0])))
	{
		s_len = sprintf(printf_buf, "%06d - ", clock());
		if (s_len > 0)
		{
			s_len = vsnprintf((char*)& printf_buf[s_len], (sizeof(printf_buf) / sizeof(printf_buf[0])) - s_len, format, args);
		}
		if (s_len > 0)
		{
			printf("%s\n", printf_buf);
		}
	}
	else
	{
		printf("printf_buf too small\n");
	}
	va_end(args);
	if (outFile != NULL)
	{
		fclose(outFile);
		fclose(stdout);
		outFile = NULL;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (AllocConsole())
		{
			HWND hwnd = GetConsoleWindow();
			hwnd = GetConsoleWindow();
			HMENU hmenu = GetSystemMenu(hwnd, FALSE);
			EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
		}
		memset(printf_buf, 0x00, sizeof(printf_buf) / sizeof(printf_buf[0]));
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
	{
		break;
	}
	}
	return TRUE;
}

