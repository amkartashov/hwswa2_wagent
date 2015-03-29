#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

// Need to link with Iphlpapi.lib and Ws2_32.lib
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#include "auxiliary.h"
#include "nc.h"
#include "debug.h"

ILIST check_ports(LPWSTR address, RANGE ports){
	// ======= convert address into in_addr structure
	size_t address_len = wcslen(address);
	char *c_address = malloc((address_len+1)*sizeof(char));
	size_t pReturnValue;
	wcstombs_s(&pReturnValue, c_address, address_len+1, address, address_len);
	u_long IP;
	u_long IP2;
	inet_pton(AF_INET,c_address, &IP);
	// ======= convert ports range into ports list
	ILIST asked_ports = range2ilist(ports);
	ilist_sort(&asked_ports);
	// ======= find listened ports which are in address:asked_ports
	ILIST listened_ports = { .arr = NULL, .len = 0, .allocated = 0 };
	PMIB_TCPTABLE pTcpTable = (MIB_TCPTABLE *) malloc(sizeof (MIB_TCPTABLE));
	DWORD dwSize = sizeof (MIB_TCPTABLE);
    if (ERROR_INSUFFICIENT_BUFFER == GetTcpTable(pTcpTable, &dwSize, TRUE)) {
        free(pTcpTable);
        pTcpTable = (MIB_TCPTABLE *) malloc(dwSize);
        GetTcpTable(pTcpTable, &dwSize, TRUE);}
    int i;
    u_short port;
    for (i=0; i<(int) pTcpTable->dwNumEntries; i++) {
    	if (MIB_TCP_STATE_LISTEN == pTcpTable->table[i].dwState) { // listening
    		IP2 = (u_long) pTcpTable->table[i].dwLocalAddr;
    	    if ( IP == IP2 || 0 == IP2 ) { // on the same IP or all IPs
    			port = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
    			if (in_ilist(asked_ports, port))
    				ilist_extend(&listened_ports, port);}}}
	// ======= free allocated resources
    free(pTcpTable);
	free(asked_ports.arr);
	free(c_address);
	return listened_ports;}
