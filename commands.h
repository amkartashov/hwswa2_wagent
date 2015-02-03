#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")

// should be not more than MAXMSGSIZE in pipefunc.h
#define CMD_RESULT_BUFSIZE 32000
#define CMD_DEF_TIMEOUT 30
#define CMD_ACCEPTED_OK TEXT("accepted_ok")
#define CMD_ACCEPTED_NOTOK TEXT("accepted_notok")
#define CMD_RESULT_OK TEXT("result_ok")
#define CMD_RESULT_NOTOK TEXT("result_notok")


extern WCHAR* BANNER;

struct Command;

struct Command * FindCommand(WCHAR* cmdline);
VOID ExecCommand(struct Command *cmd, WCHAR* cmdline, WCHAR* result, HANDLE stopevent, int timeout);