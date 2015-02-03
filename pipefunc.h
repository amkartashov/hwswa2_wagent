#pragma once

#define MAXMSGSIZE 32000

extern BOOL CMDPIPEconnected;

HANDLE CreateCMDPipe(VOID);
BOOL ConnectOrStop(HANDLE hPipe, LPOVERLAPPED lpo, HANDLE stopevent);
BOOL SendOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent);
BOOL ReceiveOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent);
