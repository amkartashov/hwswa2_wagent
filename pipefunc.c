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
#include "auxiliary.h"
#include "pipefunc.h"

#define CMD_PIPE TEXT("\\\\.\\pipe\\hwswa2_agent")
//#define STDIN_PIPE TEXT("\\\\.\\pipe\\hwswa2_stdin")
//#define STDOUT_PIPE TEXT("\\\\.\\pipe\\hwswa2_stdout")
//#define STDERR_PIPE TEXT("\\\\.\\pipe\\hwswa2_stderr")
#define PIPE_BUFSIZE 4096
#define PIPE_TIMEOUT 5000 // 5 seconds

BOOL CMDPIPEconnected = FALSE;

// ============ named pipe functions

HANDLE CreateCMDPipe(VOID){
    return CreateNamedPipe(CMD_PIPE,
                            PIPE_ACCESS_DUPLEX |     // read/write access
                            FILE_FLAG_OVERLAPPED,    // overlapped mode
                            PIPE_TYPE_MESSAGE |      // message-type pipe
                            PIPE_READMODE_MESSAGE |  // message-read mode
                            PIPE_WAIT,               // blocking mode
                            1,                       // number of instances
                            PIPE_BUFSIZE*sizeof(WCHAR),   // output buffer size
                            PIPE_BUFSIZE*sizeof(WCHAR),   // input buffer size
                            PIPE_TIMEOUT,            // client time-out
                            NULL);                   // default security attributes
}

BOOL ConnectOrStop(HANDLE hPipe, LPOVERLAPPED lpo, HANDLE stopevent){ 
	if (signalled(stopevent)) return FALSE;
    debug(TEXT("Trying to connect"));
	// Initiate connection
	if (ConnectNamedPipe(hPipe, lpo)){
    	debug(TEXT("ConnectNamedPipe failed")); 
      	return FALSE;}
   	switch (GetLastError()){
   		// The overlapped connection in progress.
 	 	case ERROR_IO_PENDING: break;
  		// Client is already connected, so signal an event.
      	case ERROR_PIPE_CONNECTED: 
            CMDPIPEconnected = TRUE;
            return TRUE;
 	    // If an error occurs during the connect operation... 
        default: 
        	debug(TEXT("ConnectNamedPipe failed (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
        	return FALSE;}
    // Now wait till connection is established or stop event is received
	HANDLE hEvents[2];
	hEvents[0] = stopevent;
	hEvents[1] = lpo->hEvent;
    switch(WaitForMultipleObjects(2, hEvents, FALSE, INFINITE)){
    	// stop event signalled
    	case WAIT_OBJECT_0:
    		debug(TEXT("Stop event signalled"));
    		return FALSE;
    	// Pipe connect event signalled
    	case WAIT_OBJECT_0 + 1:
    		debug(TEXT("Connected"));
            CMDPIPEconnected = TRUE;
    		return TRUE;
    	// some failure happened
    	case WAIT_FAILED:
    		debug(TEXT("Failure while waiting for connection"));}
    return FALSE;}


BOOL SendOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent){
	DWORD bytes_written;
	if (signalled(stopevent) || !CMDPIPEconnected) return FALSE;
    debug(TEXT("About to send: %s"), msg);
	// Initiate writing
	if (WriteFile(hPipe, msg, wcslen(msg)*sizeof(WCHAR), NULL, lpo)){
    	debug(TEXT("WriteFile successed without delay")); 
      	return TRUE;}
   	switch (GetLastError()){
   		// The overlapped write in progress.
 	 	case ERROR_IO_PENDING: break;
  		// Some failure has happened
  		case ERROR_NO_DATA: // The pipe is being closed.
  			CMDPIPEconnected = FALSE;
        default:
        	debug(TEXT("WriteFile failed (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
    		return FALSE;}
    // Now wait till writefile has finished or stop event is received
	HANDLE hEvents[2];
	hEvents[0] = stopevent;
	hEvents[1] = lpo->hEvent;
    switch(WaitForMultipleObjects(2, hEvents, FALSE, INFINITE)){
    	// stop event signalled
    	case WAIT_OBJECT_0:
    		debug(TEXT("Stop event signalled"));
    		return FALSE;
    	// Pipe connect event signalled
    	case WAIT_OBJECT_0 + 1:
    		debug(TEXT("WriteFile finished"));
    		if (GetOverlappedResult(hPipe, lpo, &bytes_written, FALSE)){
    			debug(TEXT("WriteFile finished OK"));
    			return TRUE;}
			else {
				debug(TEXT("WriteFile failed (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
				return FALSE;}
    		break;
    	// some failure happened
    	case WAIT_FAILED:
    		debug(TEXT("Failure while waiting for WriteFile (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));}
    return FALSE;}


BOOL ReceiveOrStop(HANDLE hPipe, LPOVERLAPPED lpo, WCHAR* msg, HANDLE stopevent){
	DWORD bytes_read;
	if (signalled(stopevent) || !CMDPIPEconnected) return FALSE;
	// Initiate reading
	if (ReadFile(hPipe, msg, MAXMSGSIZE*sizeof(WCHAR), NULL, lpo)){
    	debug(TEXT("ReadFile successed without delay")); 
      	return TRUE;}
   	switch (GetLastError()){
   		// The overlapped read in progress.
 	 	case ERROR_IO_PENDING: break;
  		// Failure.
  		case ERROR_BROKEN_PIPE:
  			CMDPIPEconnected = FALSE;
        default: 
        	debug(TEXT("ReadFile failed (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
        	return FALSE;}
    // Now wait till readfile has finished or stop event is received
	HANDLE hEvents[2];
	hEvents[0] = stopevent;
	hEvents[1] = lpo->hEvent;
    switch(WaitForMultipleObjects(2, hEvents, FALSE, INFINITE)){
    	// stop event signalled
    	case WAIT_OBJECT_0:
    		debug(TEXT("Stop event signalled"));
    		return FALSE;
    	// Pipe connect event signalled
    	case WAIT_OBJECT_0 + 1:
    		if (GetOverlappedResult(hPipe, lpo, &bytes_read, FALSE)){
    			msg[bytes_read/sizeof(WCHAR)] = 0;
    			debug(TEXT("ReadFile finished, msg(%d bytes): %s"), bytes_read, msg);
    			return TRUE;}
    		else {
    			debug(TEXT("ReadFile failed (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
    			if (ERROR_BROKEN_PIPE == GetLastError()) CMDPIPEconnected = FALSE;
    			return FALSE;}
    		break;
    	// some failure happened
    	case WAIT_FAILED:
    		debug(TEXT("Failure while waiting for ReadFile (%d): %s"), GetLastError(), ErrorMessage(GetLastError()));
    		break;}
    return FALSE;}
