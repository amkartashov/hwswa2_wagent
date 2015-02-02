#include "commands.h"

WCHAR* BANNER = TEXT("started_ok possible commands: exit");

struct Command {
	WCHAR* cmdname;
	WCHAR* (*cmdfunc)(WCHAR* args, HANDLE stopevent, HANDLE timeoutevent);
};

// aux functions declarations

WCHAR* get_cmdname(WCHAR* cmdline);
WCHAR* get_argsline(WCHAR* cmdline);

// command function declarations

WCHAR* cmd_exit(WCHAR* args, HANDLE stopevent, HANDLE timeoutevent);

struct Command commands[1] = { {TEXT("exit"), cmd_exit }, };

struct Command * FindCommand(WCHAR* cmdline);
WCHAR* ExecCommand(struct Command *cmd, WCHAR* cmdline, HANDLE stopevent, int timeout);


// command functions

WCHAR* cmd_exit(WCHAR* args, HANDLE stopevent, HANDLE timeoutevent){
	SetEvent(stopevent);
	return CMD_RESULT_OK;}
