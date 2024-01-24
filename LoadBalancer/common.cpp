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

static unsigned int worker_process_count = 0;

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

void createWorker() {
    worker_node* new_worker = (worker_node*)malloc(sizeof(worker_node));

    memset(&new_worker->startup_info, 0, sizeof(STARTUPINFO));
    new_worker->startup_info.cb = sizeof(STARTUPINFO);
    new_worker->startup_info.dwFlags = STARTF_USESHOWWINDOW;
    new_worker->startup_info.wShowWindow = SW_SHOW;

    memset(&new_worker->process_info, 0, sizeof(PROCESS_INFORMATION));
    //TCHAR buff[100];
    //GetCurrentDirectory(100, buff);
    //wcscat(buff, L"\\..\\Debug\\Worker.exe");
    //wcscat_s(buff, L"\\..\\Client\\x64\\Debug\\Worker.exe");
    wchar_t buff[] = L"C:\\Users\\bugar\\OneDrive\\Desktop\\IKPPROJEKAT\\Projekat_IKP\\Client\\x64\\Debug\\Worker.exe";
    TCHAR cmd[] = L"Worker.exe";
    if (!CreateProcess(
        buff,          // LPCTSTR lpApplicationName
        cmd,                // LPTSTR lpCommandLine
        NULL,                // LPSECURITY_ATTRIBUTES lpProcessAttributes
        NULL,                // LPSECURITY_ATTRIBUTES lpThreadAttributes
        FALSE,               // BOOL bInheritHandles
        NORMAL_PRIORITY_CLASS,    // DWORD dwCreationFlags
        NULL,                // LPVOID lpEnvironment
        NULL,                // LPCTSTR lpCurrentDirectory
        &new_worker->startup_info,       // LPSTARTUPINFO lpStartupInfo
        &new_worker->process_info        // LPPROCESS_INFORMATION lpProcessInformation
    )) {

        printf("CreateProcess failed (%d).\n", GetLastError());
    }

    worker_process_count++;
    insert_worker_node(new_worker);
    ShowWindow(0, SW_SHOW);
}

CRITICAL_SECTION globalCs;

void deleteFreeWorker() {
    node* first_elem = delete_first_node(free_workers_list);

    if (first_elem != NULL) {

        if (first_elem->msgStruct == NULL)
            first_elem->msgStruct = (messageStruct*)malloc(sizeof(messageStruct));

        strcpy_s(first_elem->msgStruct->bufferNoName, "exit");
        ReleaseSemaphore(first_elem->msgSemaphore, 1, NULL);
        if (first_elem->thread_read)
            WaitForSingleObject(first_elem->thread_read, INFINITE);
        if (first_elem->thread_write)
            WaitForSingleObject(first_elem->thread_write, INFINITE);
        //free(first_elem->msgStruct);
        free(first_elem);

        worker_process_count--;
    }
}

DWORD WINAPI check_percentage(LPVOID param) {
    while (true) {
        if (WaitForSingleObject(semaphoreEnd, 10) == WAIT_OBJECT_0)
            break;
        Sleep(1000);
        int fullfillness = ((float)get_current_size_queue() / (float)get_capacity_queue()) * 100;
        printf("Queue is at %d%%\n", fullfillness);
        if (fullfillness < 30 && worker_process_count>1) {
            //shut down worker threads
            deleteFreeWorker();

        }
        else if (fullfillness > 70) {
            // open new worker processes
            createWorker();
        }
    }
    return 0;
}

DWORD WINAPI dispatcher(LPVOID param) {

    messageStruct* dequeuedMessageStruct = NULL;

    while (true) {
        if (WaitForSingleObject(semaphoreEnd, 10) == WAIT_OBJECT_0)
            break;
        Sleep(50);

        if (!is_queue_empty()) {

            node* first = free_workers_list->head;

            if (free_workers_list->head != NULL)
            {

#ifdef DEBUG_LIST
                printf("Free Worker ");
                print_list(free_workers_list);
#endif
                dequeue(&dequeuedMessageStruct);

                EnterCriticalSection(&free_workers_list->cs);
                first->msgStruct = dequeuedMessageStruct;
                ReleaseSemaphore(first->msgSemaphore, 1, NULL);
                LeaveCriticalSection(&free_workers_list->cs);

                move_first_node(busy_workers_list, free_workers_list);
#ifdef DEBUG_LIST
                printf("Busy Worker ");
                print_list(busy_workers_list);
#endif
            }
        }
    }
    return 0;
}
