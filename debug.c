#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")


#ifdef DEBUG
#define LOGFILE TEXT("hwswa2_agent.log")
#define MAXMSGLEN 32767
HANDLE debuglog;
#endif

void prepare_log(void){
#ifdef DEBUG
	WCHAR logfilepath[MAX_PATH+1];
	GetEnvironmentVariable(TEXT("TEMP"), logfilepath, MAX_PATH+1);
	wcscat_s(logfilepath, MAX_PATH+1, TEXT("\\"));
	wcscat_s(logfilepath, MAX_PATH+1, LOGFILE);
	debuglog = CreateFile(logfilepath, FILE_APPEND_DATA , FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
#endif
}

void debug(WCHAR* msgfmt, ...){
#ifdef DEBUG
	DWORD bytes_written;
	// constructed message
	WCHAR msg[MAXMSGLEN];
	va_list args;
    va_start(args, msgfmt);
    vswprintf_s(msg, MAXMSGLEN, msgfmt, args);
    va_end(args);
	// message with time
	WCHAR resultmsg[MAXMSGLEN];
	SYSTEMTIME now;
	WCHAR time[4096];
	resultmsg[0] = 0;
	GetLocalTime(&now);
	swprintf(time, 4096, TEXT("%u/%02u/%02u %02u:%02u:%02u:%03u"),
			 now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute,
			 now.wSecond, now.wMilliseconds);
	wcscat_s(resultmsg, MAXMSGLEN, time);
	wcscat_s(resultmsg, MAXMSGLEN, L" ");
	wcscat_s(resultmsg, MAXMSGLEN, msg);
	wcscat_s(resultmsg, MAXMSGLEN, L"\r\n");
	WriteFile(debuglog, resultmsg, sizeof(WCHAR)*wcslen(resultmsg), &bytes_written, NULL);
#endif
}

void close_log(void){
#ifdef DEBUG
	CloseHandle(debuglog);
#endif
}

