#include <stdlib.h>
#include <stdio.h>
#include "hashmap.h"
#include "queue.h"
#include "client_communication.h"
#include "worker_communication.h"
#include "list.h"
#include "workers.h"
#include "test.h"

#define WORKER_IP_ADDRESS "127.0.0.1"
#define WORKER_PORT 6069
//#define DEBUG_LIST
#define DEBUG_QUEUE

#pragma warning(disable:4996)

list* busy_workers_list;
list* free_workers_list;
queue* q;
HANDLE semaphoreEnd;
worker_node* worker_process_head = NULL;




int main() {
    int a = 0;
    HANDLE hListenerClient;
    HANDLE hListenerWorker;
    HANDLE hPercentage;
    HANDLE hDispatcher;

    DWORD listenerClientID;
    DWORD percentageID;
    DWORD listenerWorkerID;
    DWORD dispatcherID;

    init_hash_table();
    create_queue(50);
    init_list(&free_workers_list);
    init_list(&busy_workers_list);
    semaphoreEnd = CreateSemaphore(0, 0, 4, NULL);


    hPercentage = CreateThread(NULL, 0, &check_percentage, (LPVOID)0, 0, &percentageID);
    hListenerClient = CreateThread(NULL, 0, &client_listener, (LPVOID)0, 0, &listenerClientID);
    hListenerWorker = CreateThread(NULL, 0, &worker_listener, (LPVOID)0, 0, &listenerWorkerID);
    hDispatcher = CreateThread(NULL, 0, &dispatcher, (LPVOID)0, 0, &dispatcherID);

    create_new_worker_process();
    //test_list();
    //test_hashtable();
    //test_dynamic_enqueue_dequeue();


    printf("Press any key to exit:\n");
    char input[2];
    gets_s(input, 2);


    //close the process and worker write and read thread
    shut_down_first_free_process();
    ReleaseSemaphore(semaphoreEnd, 4, NULL);

    //wait for listener to finish
    if (hPercentage)
        WaitForSingleObject(hPercentage, INFINITE);
    if (hDispatcher)
        WaitForSingleObject(hDispatcher, INFINITE);
    if (hListenerWorker)
        WaitForSingleObject(hListenerWorker, INFINITE);
    if (hListenerClient)
        WaitForSingleObject(hListenerClient, INFINITE);

    delete_hashtable();
    delete_list(free_workers_list);
    delete_list(busy_workers_list);
    delete_queue();
    delete_worker_list_and_shutdown();

    return 0;
}

