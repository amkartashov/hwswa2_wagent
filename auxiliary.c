#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")

// Retrieve the system error message for the last-error code
LPCWSTR ErrorMessage(DWORD error){ 
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    			  NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR) &lpMsgBuf, 0, NULL );
    return((LPCWSTR)lpMsgBuf);
}


BOOL signalled(HANDLE manual_reset_event){
	return WaitForSingleObjectEx(manual_reset_event, 0, TRUE) != WAIT_TIMEOUT;
}


