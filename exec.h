#pragma once

#define EXEC_MAX_REASON 4096
#define EXEC_MAX_CMDLINE 32767
#define EXEC_MAX_IN 32767
#define EXEC_MAX_OUT 10000

//timeout in seconds, 0 means infinite
BOOL Execute(WCHAR *cmdline, WCHAR *in, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout);

BOOL Execute_CMD(WCHAR *cmdline, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout);

BOOL Execute_PSE(WCHAR *encoded_cmd, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout);