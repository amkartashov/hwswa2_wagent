#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "advapi32.lib")

#include "debug.h"
#include "selfdelete.h"
#include "pipefunc.h"
#include "auxiliary.h"
#include "commands.h"

// ========== Global variables
#define SVCNAME TEXT("hwswa2_agent")

SERVICE_STATUS gSvcStatus; 
SERVICE_STATUS_HANDLE gSvcStatusHandle; 
HANDLE ghSvcStopEvent = NULL;

// ========== Function declarations

// service functions
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv); 
VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl);
VOID cleanup(VOID);

// ========== MAIN

int wmain(int argc, WCHAR *argv[]){
	SelfDelete(TRUE);
	prepare_log();
    debug(TEXT("about to start"));
    debug(TEXT("init services table"));
    SERVICE_TABLE_ENTRY service_table[2];
    service_table[0].lpServiceName = SVCNAME;
    service_table[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION) SvcMain;
    service_table[1].lpServiceName = NULL;
    service_table[1].lpServiceProc = NULL;
    debug(TEXT("about to start service dispatcher"));
    if (!StartServiceCtrlDispatcher(service_table)){ 
        debug(TEXT("Failed to start service dispatcher"));
    	cleanup();}
    return 0;}


// service functions

VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv){
    gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
    if(!gSvcStatusHandle){ 
        debug(TEXT("RegisterServiceCtrlHandler failed"));
        return;} 
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
    SvcInit(dwArgc, lpszArgv);
	cleanup();}


VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv){
	// Prepare events, service stop event and command pipe event (inside OVERLAPPED)
    ghSvcStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ghSvcStopEvent == NULL){
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        debug(TEXT("CreatEvent failed for ghSvcStopEvent"));
        return;}
	OVERLAPPED oOverlap;
	oOverlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (oOverlap.hEvent == NULL){
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        debug(TEXT("CreatEvent failed for oOverlap.hEvent"));
        return;}
	HANDLE cmdPipe = INVALID_HANDLE_VALUE;
    cmdPipe = CreateCMDPipe();
    if (INVALID_HANDLE_VALUE == cmdPipe){
    	debug(TEXT("Failed to create command pipe"));	
    	return;
    }
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
    debug(TEXT("Init done"));
    while(!signalled(ghSvcStopEvent)){
    	debug(TEXT("Trying to connect"));
	    ConnectOrStop(cmdPipe, &oOverlap, ghSvcStopEvent);
	    if (CMDPIPEconnected){
	    	WCHAR msg[MAXMSGSIZE];
	    	wcscpy_s(msg, MAXMSGSIZE, BANNER);
	    	debug(TEXT("About to send: %s"), msg);
	    	SendOrStop(cmdPipe, &oOverlap, msg, ghSvcStopEvent);
	    	while(CMDPIPEconnected && !signalled(ghSvcStopEvent)){
	    		// read command
	    		if (ReceiveOrStop(cmdPipe, &oOverlap, msg, ghSvcStopEvent)) {
		    		debug(TEXT("Received: %s"), msg);
		    		// execute command
		    		// send reply
			    	wcscpy_s(msg, MAXMSGSIZE, TEXT("result_ok"));
			    	debug(TEXT("About to send: %s"), msg);
		    		SendOrStop(cmdPipe, &oOverlap, msg, ghSvcStopEvent);}}
	    	debug(TEXT("Will disconnect"));
			DisconnectNamedPipe(cmdPipe);}}
   	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
	return;}


VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint){
    static DWORD dwCheckPoint = 1;
    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;
    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else
    	gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED)){
        gSvcStatus.dwCheckPoint = 0;
    	dwCheckPoint = 1;}
    else
    	gSvcStatus.dwCheckPoint = dwCheckPoint++;
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);}


VOID WINAPI SvcCtrlHandler(DWORD dwCtrl){
   switch(dwCtrl){  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         return;
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
      default: 
         break;}}


VOID cleanup(){
	debug(TEXT("cleanup() called"));
	if(DeleteService(OpenService(OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS),
								 SVCNAME, SC_MANAGER_ALL_ACCESS)))
		debug(TEXT("cleanup: Service deleted"));
	else
		debug(TEXT("cleanup: FAILED to delete service"));
	debug(TEXT("cleanup: about to close log"));
	close_log();}


