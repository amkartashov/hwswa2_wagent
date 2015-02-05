#include "commands.h"
#include "debug.h"
#include "base64.h"

// DO NOT FORGET TO INCREASE IF ADDING NEW COMMAND
#define NUMBER_OF_CMDS 3

// DO NOT FORGET TO ADD COMMAND TO BANNER IF ADDING NEW COMMAND
WCHAR* BANNER = TEXT("started_ok possible commands: exit, base64enc, base64dec");

struct Command {
	WCHAR* cmdname;
	VOID (*cmdfunc)(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);};

// command function declarations and commands array

VOID cmd_exit(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_base64enc(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);
VOID cmd_base64dec(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);

struct Command commands[NUMBER_OF_CMDS] = {
    {TEXT("exit"), cmd_exit }, 
    {TEXT("base64enc"), cmd_base64enc },
    {TEXT("base64dec"), cmd_base64dec },
};

// FindCommand and ExecCommand

struct Command * FindCommand(WCHAR* cmdline){
	int i;
	for (i=0; i<NUMBER_OF_CMDS; i++)
		if (0 == wcsncmp(cmdline, commands[i].cmdname, wcslen(commands[i].cmdname))){
			debug(TEXT("Found command: %s"), commands[i].cmdname);
			return &commands[i];}
	debug(TEXT("Command not found: %s"), cmdline);
	return NULL;}

VOID ExecCommand(struct Command *cmd, WCHAR* cmdline, WCHAR* result, HANDLE stopevent, int timeout){
	WCHAR* args = cmdline + wcslen(cmd->cmdname) + 1;
	cmd->cmdfunc(args, result, stopevent, timeout);}

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