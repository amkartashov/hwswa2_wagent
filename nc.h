#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define NC_MAX_SOCKETS 4096
#define PROTO_TCP 1
#define PROTO_UDP 2
#define NC_DEFAULT_MESSAGE TEXT("from_send")


#include "auxiliary.h"

// check which TCP ports are listening and returns list of them
ILIST check_ports(LPWSTR address, RANGE ports);

BOOL start_listening(int proto, LPWSTR address, RANGE ports);
BOOL stop_listening(int proto, LPWSTR address, RANGE ports);
BOOL stop_listening_all();
BOOL send_to(int proto, LPWSTR address, RANGE ports, LPWSTR message, int timeout);
BOOL receive_from(int proto, LPWSTR address, RANGE ports, LPWSTR message, int timeout);
