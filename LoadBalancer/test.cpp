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

    printf("Pre zauzimanja memorije");
    getchar();


    init_list(&listSlobodni);
    init_list(&listZauzeti);

    insert_last_node(freeWorker1, listSlobodni);
    insert_first_node(freeWorker2, listSlobodni);



    insert_last_node(busyWorker1, listZauzeti);
    insert_last_node(busyWorker2, listZauzeti);

    printf("Zauzeta mem");
    getchar();
    getchar();
  

    printf("\nLista slobodnih: ");
    print_list(listSlobodni);
    delete_node(freeWorker1, listSlobodni);

    printf("\nLista slobodnih nakon brisanja jednog: ");
    print_list(listSlobodni);

    delete_node(freeWorker2, listSlobodni);
    printf("\nLista slobodnih nakon brisanja svih: ");
    print_list(listSlobodni);

    printf("\nLista zauzetih: ");
    print_list(listZauzeti);

    delete_node(busyWorker1, listZauzeti);
    delete_node(busyWorker2, listZauzeti);

    printf("\nLista zauzetih nakon brisanja svih: ");
    print_list(listZauzeti);

    printf("Oslobodjena mem");
    delete_list(listSlobodni);
    delete_list(listZauzeti);
}

void test_hashtable() {

    DWORD h1,h2,h3,h4,h5,h6;



    client_thread* test1 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t1 = CreateThread(NULL, 0, &test, (LPVOID)test1, 0, &h1);
    test1->acceptedSocket = NULL;
    test1->clientThread = t1;
    test1->finished = true;
    strcpy(test1->clientName, "Client1");
    
    client_thread* test2 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t2 = CreateThread(NULL, 0, &test, (LPVOID)test2, 0, &h2);
    test1->acceptedSocket = NULL;
    test1->clientThread = t2;
    test1->finished = true;
    strcpy(test2->clientName, "Client2");
    
    client_thread* test3 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t3 = CreateThread(NULL, 0, &test, (LPVOID)test3, 0, &h3);
    test1->acceptedSocket = NULL;
    test1->clientThread = t3;
    test1->finished = true;
    strcpy(test3->clientName, "Client3");
    
    client_thread* test4 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t4 = CreateThread(NULL, 0, &test, (LPVOID)test4, 0, &h4);
    test4->acceptedSocket = NULL;
    test4->clientThread = t4;
    test4->finished = true;
    strcpy(test4->clientName, "Client4");
    
    client_thread* test5 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t5 = CreateThread(NULL, 0, &test, (LPVOID)test5, 0, &h5);
    test5->acceptedSocket = NULL;
    test5->clientThread = t5;
    test5->finished = true;
    strcpy(test5->clientName, "Client5");
    
    client_thread* test6 = (client_thread*)malloc(sizeof(client_thread));
    HANDLE t6 = CreateThread(NULL, 0, &test, (LPVOID)test6, 0, &h6);
    test6->acceptedSocket = NULL;
    test6->clientThread = t6;
    test6->finished = true;
    strcpy(test6->clientName, "Client6");



    printf("Pre zauzimanja mem");
    getchar();
    getchar();

    //init_hash_table();

    insert_client(test1);
    insert_client(test2);
    insert_client(test3);
    insert_client(test4);
    insert_client(test5);
    insert_client(test6);

    print_table();
}

DWORD WINAPI add_msg_to_queue(LPVOID param) {
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
DWORD WINAPI dequeue_message(LPVOID param) {
    queue* q = (queue*)param;
    while (true) {
        messageStruct* dequeuedMessageStruct = NULL;
        Sleep(5000);
        dequeue(&dequeuedMessageStruct);
    }
   

}
void test_messages() {
    //printf("Pre zauzimanja queue");
    //getchar();
    //getchar();


    //create_queue(10);


    printf("Nakon zauzimanja queue");
    getchar();
    getchar();



    HANDLE hProducer;
    HANDLE hConsumer;
    DWORD ProducerID;
    DWORD ConsumerID;

    hProducer = CreateThread(NULL, 0, &add_msg_to_queue, (LPVOID)q, 0, &ProducerID);
    hConsumer = CreateThread(NULL, 0, &dequeue_message, (LPVOID)q, 0, &ConsumerID);

    if (hProducer)
        WaitForSingleObject(hProducer, INFINITE);
    if (hConsumer)
        WaitForSingleObject(hConsumer, INFINITE);
    
}