#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include "common.h"
#include "hashmap.h"
#include "queue.h"
#include "client_communication.h"
#include "worker_communication.h"
#include "list.h"
#include "workers.h"



bool is_socket_ready(SOCKET socket, bool isRead) {
	FD_SET set;
	timeval tv;

	FD_ZERO(&set);
	FD_SET(socket, &set);

	tv.tv_sec = 0;
	tv.tv_usec = 50;

	int iResult;

	if (isRead) { //see if socket is ready for READ
		iResult = select(0, &set, NULL, NULL, &tv);
	}
	else {	//see if socket is ready for WRITE
		iResult = select(0, NULL, &set, NULL, &tv);
	}

	if (iResult <= 0)
		return false;
	else
		return true;
}
