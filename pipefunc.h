#pragma once

#define MAXMSGSIZE 4096

extern BOOL CMDPIPEconnected;

// pipe functions
HANDLE CreateCMDPipe(VOID);
BOOL ConnectOrStop(HANDLE hPipe, LPOVERLAPPED lpo, HANDLE stopevent);
BOOL SendOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent);
BOOL ReceiveOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent);
