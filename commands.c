#include "commands.h"
#include "debug.h"

// DO NOT FORGET TO INCREASE IF ADDING NEW COMMAND
#define NUMBER_OF_CMDS 1

// DO NOT FORGET TO ADD COMMAND TO BANNER IF ADDING NEW COMMAND
WCHAR* BANNER = TEXT("started_ok possible commands: exit");

struct Command {
	WCHAR* cmdname;
	VOID (*cmdfunc)(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);};

// command function declarations and commands array

VOID cmd_exit(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout);

struct Command commands[NUMBER_OF_CMDS] = { {TEXT("exit"), cmd_exit }, };

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
	WCHAR* args = cmdline + wcslen(cmd->cmdname);
	cmd->cmdfunc(args, result, stopevent, timeout);}

// command functions

VOID cmd_exit(WCHAR* args, WCHAR* result, HANDLE stopevent, int timeout){
	SetEvent(stopevent);
	wcscpy_s(result, CMD_RESULT_BUFSIZE, CMD_RESULT_OK);}
