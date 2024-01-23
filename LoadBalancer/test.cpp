#include "test.h"
#include "queue.h"
#include "hashmap.h"
#include "list.h"
#include "common.h"
#pragma warning(disable:4996)


static CRITICAL_SECTION testCS;

DWORD WINAPI test(LPVOID param) {

    printf("Created test thread\n");
    return 0;

}

void test_list() {

   
    DWORD hFree1ID, hFree2ID, hFree3ID, hBusy1ID, hBusy2ID, hBusy3ID;
   



    node* freeWorker1 = (node*)malloc(sizeof(node));
    freeWorker1->msgSemaphore = CreateSemaphore(0, 0, 1, NULL);
    freeWorker1->msgStruct = NULL;
    freeWorker1->acceptedSocket = NULL;
    freeWorker1->next = NULL;

    HANDLE t1 = CreateThread(NULL, 0, &test, (LPVOID)freeWorker1, 0, &hFree1ID);
    if (!SetThreadDescription(t1, L"Slobodni Thread 1")) {
        printf("Error setting thread description: %lu\n", GetLastError());
    }

    freeWorker1->thread_write = t1;
    freeWorker1->thread_read = t1;

    node* freeWorker2 = (node*)malloc(sizeof(node));
    freeWorker2->msgSemaphore = CreateSemaphore(0, 0, 1, NULL);
    freeWorker2->msgStruct = NULL;
    freeWorker2->acceptedSocket = NULL;
    freeWorker2->next = NULL;

    HANDLE t2 = CreateThread(NULL, 0, &test, (LPVOID)freeWorker2, 0, &hFree2ID);
    if (!SetThreadDescription(t2, L"Slobodni Thread 2")) {
        printf("Error setting thread description: %lu\n", GetLastError());
    }
    freeWorker2->thread_write = t2;
    freeWorker2->thread_read = t2;

    node* busyWorker1 = (node*)malloc(sizeof(node));
    busyWorker1->msgSemaphore = CreateSemaphore(0, 0, 1, NULL);
    busyWorker1->msgStruct = NULL;
    busyWorker1->acceptedSocket = NULL;
    busyWorker1->next = NULL;

    HANDLE t3 = CreateThread(NULL, 0, &test, (LPVOID)busyWorker1, 0, &hBusy1ID);
    if (!SetThreadDescription(t3, L"Zauzet Thread 1")) {
        printf("Error setting thread description: %lu\n", GetLastError());
    }
    busyWorker1->thread_write = t3;
    busyWorker1->thread_read = t3;

    node* busyWorker2 = (node*)malloc(sizeof(node));
    busyWorker2->msgSemaphore = CreateSemaphore(0, 0, 1, NULL);
    busyWorker2->msgStruct = NULL;
    busyWorker2->acceptedSocket = NULL;
    busyWorker2->next = NULL;


    HANDLE t4 = CreateThread(NULL, 0, &test, (LPVOID)busyWorker2, 0, &hBusy2ID);
    if (!SetThreadDescription(t4, L"Zauzet Thread 2")) {
        printf("Error setting thread description: %lu\n", GetLastError());
    }
    busyWorker2->thread_write = t4;
    busyWorker2->thread_read = t4;

    list* listSlobodni = NULL;
    list* listZauzeti = NULL;

    Sleep(3000);

    init_list(&listSlobodni);
    init_list(&listZauzeti);

    insert_last_node(freeWorker1, listSlobodni);
    insert_first_node(freeWorker2, listSlobodni);



    insert_last_node(busyWorker1, listZauzeti);
    insert_last_node(busyWorker2, listZauzeti);


  

    printf("\nLista slobodnih: ");
    print_list(listSlobodni);
    delete_node(freeWorker1, listSlobodni);

    printf("\nLista slobodnih nakon brisanja: ");
    print_list(listSlobodni);

    delete_node(freeWorker2, listSlobodni);
    printf("\nLista slobodnih nakon brisanja: ");
    print_list(listSlobodni);

    printf("\nLista zauzetih: ");
    print_list(listZauzeti);

    delete_node(busyWorker1, listZauzeti);
    delete_node(busyWorker2, listZauzeti);

    printf("\nLista zauzetih nakon brisanja: ");
    print_list(listZauzeti);
}

void test_hashtable() {
    client_thread cl1, cl2, cl3, cl4, cl5;
    strcpy(cl1.clientName, "Client1");
    strcpy(cl2.clientName, "Client2");
    strcpy(cl3.clientName, "Client3");
    strcpy(cl4.clientName, "Client4");
    strcpy(cl5.clientName, "Client5");


    init_hash_table();

    insert_client(&cl1);
    insert_client(&cl2);
    insert_client(&cl3);
    insert_client(&cl4);
    insert_client(&cl5);

    print_table();
}

DWORD WINAPI producer(LPVOID param) {
    queue* q = (queue*)param;
    char clientName[10] = "Client0";
    char message[246] = "10";
    messageStruct* msgTest = (messageStruct*)malloc(sizeof(messageStruct));

    strcpy(msgTest->clientName, clientName);
    strcpy(msgTest->bufferNoName, message);

    while (true) {
        enqueue(msgTest);
        Sleep(2000);
    }
}
DWORD WINAPI consumer(LPVOID param) {
    queue* q = (queue*)param;
    while (true) {
        messageStruct* dequeuedMessageStruct = NULL;
        Sleep(5000);
        dequeue(&dequeuedMessageStruct);
    }
}
void test_dynamic_enqueue_dequeue() {
    create_queue(10);

    HANDLE hProducer;
    HANDLE hConsumer;
    DWORD ProducerID;
    DWORD ConsumerID;

    hProducer = CreateThread(NULL, 0, &producer, (LPVOID)q, 0, &ProducerID);
    hConsumer = CreateThread(NULL, 0, &consumer, (LPVOID)q, 0, &ConsumerID);

    if (hProducer)
        WaitForSingleObject(hProducer, INFINITE);
    if (hConsumer)
        WaitForSingleObject(hConsumer, INFINITE);
    
}