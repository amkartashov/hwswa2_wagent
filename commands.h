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

#define CMD_ACCEPTED_OK TEXT("accepted_ok")
#define CMD_ACCEPTED_NOTOK TEXT("accepted_notok")
#define CMD_RESULT_OK TEXT("result_ok")
#define CMD_RESULT_NOTOK TEXT("result_notok")

extern WCHAR* BANNER;

struct Command;

struct Command * FindCommand(WCHAR* cmdline);
WCHAR* ExecCommand(struct Command *cmd, WCHAR* cmdline, HANDLE stopevent, int timeout);