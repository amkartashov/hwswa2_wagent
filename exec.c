#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include "exec.h"
#include "base64.h"
#include "debug.h"
#include "auxiliary.h"

struct Params {
	HANDLE pipe;
	WCHAR *in_or_out;
	size_t max_out_size;
};

DWORD WINAPI ReadFromPipe(LPVOID params);
DWORD WINAPI WriteToPipe(LPVOID params);


BOOL Execute_CMD(WCHAR *cmdline, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout){
	WCHAR newcmdline[EXEC_MAX_CMDLINE];
	WCHAR *cursor = newcmdline;
	size_t len;
	wcscpy_s(cursor, EXEC_MAX_CMDLINE, TEXT("cmd.exe /Q /U /C \""));
	len = wcslen(newcmdline);
	cursor = newcmdline + len;
	wcscpy_s(cursor, EXEC_MAX_CMDLINE - len, cmdline);
	len = wcslen(newcmdline);
	cursor = newcmdline + len;
	wcscpy_s(cursor, EXEC_MAX_CMDLINE - len, TEXT("\""));
	return Execute(newcmdline, TEXT(""), out, err, max_out_size, 
		           exitcode, reason, stopevent, timeout);}


BOOL Execute_PSE(WCHAR *encoded_cmd, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout){
	WCHAR newcmdline[EXEC_MAX_CMDLINE];
	WCHAR *cursor = newcmdline;
	BOOL success;
	size_t len;
	CHAR ch_out[EXEC_MAX_OUT];
	CHAR ch_err[EXEC_MAX_OUT];
	wcscpy_s(cursor, EXEC_MAX_CMDLINE, TEXT("powershell.exe -encodedCommand "));
	len = wcslen(newcmdline);
	cursor = newcmdline + len;
	wcscpy_s(cursor, EXEC_MAX_CMDLINE - len, encoded_cmd);
	success = Execute(newcmdline, TEXT(""), (WCHAR *) ch_out, (WCHAR *) ch_err, max_out_size, 
		           exitcode, reason, stopevent, timeout);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, out, max_out_size, ch_out, _TRUNCATE);
	mbstowcs_s(&convertedChars, err, max_out_size, ch_err, _TRUNCATE);
	return success;}


BOOL Execute(WCHAR *cmdline, WCHAR *in, WCHAR *out, WCHAR *err, size_t max_out_size, 
	DWORD *exitcode, WCHAR *reason, HANDLE stopevent, int timeout){

	BOOL result = TRUE;
	HANDLE stdout_r, stdout_w, stderr_r, stderr_w, stdin_w, stdin_r;
	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	DWORD last_error;
	DWORD mtimeout; // in milliseconds
	HANDLE threads[3];
	struct Params stdout_thr_args, stdin_thr_args, stderr_thr_args;
	HANDLE events[2]; // process, stopevent
	DWORD event;

	if (0 != timeout) mtimeout = 1000 * timeout;
	else mtimeout = INFINITE;

	// prepare pipes
    sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
    sa.bInheritHandle = TRUE; 
    sa.lpSecurityDescriptor = NULL; 
	CreatePipe(&stdout_r, &stdout_w, &sa, 0);
	SetHandleInformation(stdout_r, HANDLE_FLAG_INHERIT, 0);
	DWORD test = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(stdout_w, &test, NULL, NULL);
	CreatePipe(&stderr_r, &stderr_w, &sa, 0);
	SetHandleInformation(stderr_r, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&stdin_r, &stdin_w, &sa, 0);
	SetHandleInformation(stdin_w, HANDLE_FLAG_INHERIT, 0);

	// create child process
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.hStdError = stderr_w;
    si.hStdOutput = stdout_w;
    si.hStdInput = stdin_r;
    si.dwFlags |= STARTF_USESTDHANDLES;
	if(!CreateProcessW(NULL, cmdline, NULL, NULL, TRUE,
					  CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
					  NULL, NULL, &si, &pi)){
		last_error = GetLastError();
		swprintf_s(reason, EXEC_MAX_REASON, 
				   TEXT("CreateProcess failed (%d): %s"), 
				   last_error, ErrorMessage(last_error));
		debug(TEXT("Execute(): %s"), reason);
		return FALSE;}
	events[0] = pi.hProcess;

	// close unneeded pipes
	CloseHandle(stdout_w);
	CloseHandle(stderr_w);
	CloseHandle(stdin_r);

	// take care about encoding
	//AttachConsole(pi.dwProcessId);
	//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleOutputCP(65001);
	//FreeConsole();


	// start threads to save output and feed input
	stdout_thr_args = (struct Params) {stdout_r, out, max_out_size};
	threads[0] = CreateThread(NULL, 0, ReadFromPipe, (LPVOID) &stdout_thr_args, 0, NULL);
	stderr_thr_args = (struct Params) {stderr_r, err, max_out_size};
	threads[1] = CreateThread(NULL, 0, ReadFromPipe, (LPVOID) &stderr_thr_args, 0, NULL);
	stdin_thr_args = (struct Params) {stdin_w, in, max_out_size};
	threads[2] = CreateThread(NULL, 0, WriteToPipe, (LPVOID) &stdin_thr_args, 0, NULL);

	// wait for (stopevent, timeout, process)
	events[1] = stopevent;
	if (NULL == stopevent) event = WaitForMultipleObjects(1, events, FALSE, mtimeout);
	else event = WaitForMultipleObjects(2, events, FALSE, mtimeout);
	if (WAIT_OBJECT_0 + 1 == event || WAIT_TIMEOUT == event){
		// if stopevent or timeout: kill child process, set reason
		if (WAIT_OBJECT_0 + 1 == event)
			swprintf_s(reason, EXEC_MAX_REASON, 
					   TEXT("Terminating process because of stop event"));
		else
			swprintf_s(reason, EXEC_MAX_REASON, 
					   TEXT("timeout"));
		debug(TEXT("Execute(): %s"), reason);
		TerminateProcess(events[0], 0);
		result = FALSE;}
	else if (WAIT_OBJECT_0 == event) {
		// if child is finished: save exitcode
		debug(TEXT("Execute(): process has finished"));
		GetExitCodeProcess(events[0], exitcode);
		result = TRUE;}
	else { // something is wrong
		last_error = GetLastError();
		debug(TEXT("Execute(): WaitForMultipleObjects (%d): %s"), last_error, ErrorMessage(last_error));
		TerminateProcess(events[0], 0);
		result = FALSE;}
	CloseHandle(events[0]);
	// wait for threads
	WaitForMultipleObjects(3, threads, TRUE, INFINITE);
	debug(TEXT("Execute(): stdout %d bytes, stderr %d bytes"), 
		  sizeof(WCHAR)*wcslen(out), sizeof(WCHAR)*wcslen(err));
	return result;}


DWORD WINAPI ReadFromPipe(LPVOID params){
	HANDLE pipe = ((struct Params *) params)->pipe;
	WCHAR *out = ((struct Params *) params)->in_or_out;
	size_t max_out_size = ((struct Params *) params)->max_out_size;
	BYTE *outbuf = (BYTE*) out;
	DWORD bytes_read;
	DWORD total_bytes_read = 0;
	DWORD last_error;
	DWORD max_bytes_to_read = (max_out_size - 1) * sizeof(WCHAR);
	for(;;){
		if (!ReadFile(pipe, outbuf, max_bytes_to_read, &bytes_read, NULL)){
			last_error = GetLastError();
			debug(TEXT("ReadFromPipe: ReadFile failed (%d): %s"), last_error, ErrorMessage(last_error));
			break;}
		else {
			max_bytes_to_read -= bytes_read;
			total_bytes_read += bytes_read;
			if (0 == max_bytes_to_read) break;
			outbuf += bytes_read;}}
	CHAR * ch_out = (CHAR *) out;
	ch_out[total_bytes_read] = 0;
	out[total_bytes_read/sizeof(WCHAR)] = 0;
	CloseHandle(pipe);
	return 0;}


DWORD WINAPI WriteToPipe(LPVOID params){
	HANDLE pipe = ((struct Params *) params)->pipe;
	WCHAR *in = ((struct Params *) params)->in_or_out;
	BYTE *inbuf = (BYTE *) in;
	DWORD bytes_written;
	DWORD bytes_to_write = wcslen(in)*sizeof(WCHAR);
	DWORD last_error;
	while(bytes_to_write > 0){
		if (!WriteFile(pipe, inbuf, bytes_to_write, &bytes_written, NULL)){
			last_error = GetLastError();
			debug(TEXT("WriteToPipe: WriteFile failed (%d): %s"), last_error, ErrorMessage(last_error));
			break;}
		else {
			bytes_to_write -= bytes_written;
			inbuf += bytes_written;}}
	CloseHandle(pipe);
	return 0;}