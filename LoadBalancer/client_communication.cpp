#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <WinSock2.h>
#include "client_communication.h"
#include "queue.h"
#include "hashmap.h"

#pragma warning(disable:4996)
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define BUFFER_SIZE 256
#define CLIENT_NAME_LEN 10 
#define SERVER_PORT 5059

static int client_count = 0;

struct clientThreadStruct {
    SOCKET acceptedSocket;
    char clientName[CLIENT_NAME_LEN];
};
//static queue* q = NULL;

DWORD WINAPI client_read_write(LPVOID param) {
    char clientName[CLIENT_NAME_LEN];
    clientThreadStruct* paramStruct = (clientThreadStruct*)param;
    sprintf(clientName, paramStruct->clientName);
    SOCKET acceptedSocket = paramStruct->acceptedSocket;
    //free(paramStruct);

    u_long non_blocking = 1;
    ioctlsocket(acceptedSocket, FIONBIO, &non_blocking);

    char dataBuffer[BUFFER_SIZE];

    int client_num = client_count++;

    //check if we got data from client or EXIT signal
    //OR if we got a message from worker
    do
    {
        while (!is_socket_ready(acceptedSocket, true)) {

        }
        // Receive data until the client shuts down the connection
        int iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);

        if (iResult != SOCKET_ERROR)
        {
            if (iResult > 0)	// Check if message is successfully received
            {
                dataBuffer[iResult] = '\0';

                if (strcmp(dataBuffer, "exit") == 0) {
                    // Connection was closed successfully
                    printf("Connection with client %d closed.\n", client_num);
                    messageStruct* newMessageStruct = (messageStruct*)malloc(sizeof(messageStruct));
                    strcpy(newMessageStruct->clientName, clientName);
                    strcpy(newMessageStruct->bufferNoName, dataBuffer);

                    enqueue(newMessageStruct);
                    break;
                }

                // Log message text
                printf("Client %d sent: %s.\n", client_num, dataBuffer);

                messageStruct* newMessageStruct = (messageStruct*)malloc(sizeof(messageStruct));
                strcpy(newMessageStruct->clientName, clientName);
                strcpy(newMessageStruct->bufferNoName, dataBuffer);

                enqueue(newMessageStruct);

            }
            else if (iResult == 0)	// Check if shutdown command is received
            {
                printf("Connection with client closed.\n");
                break;
            }
        }
        else
        {

            if (WSAGetLastError() == WSAEWOULDBLOCK) { //the recieve would block, continue
                continue;
            }
            else {
                printf("[WORKER READ]: recv failed with error: %d\n", WSAGetLastError());
                break;
            }

        }

    } while (true);

    int iResult = shutdown(acceptedSocket, SD_BOTH);

    //// Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
        return 1;
    }

    closesocket(acceptedSocket);

    return 0;
}


DWORD WINAPI client_listener(LPVOID param) {
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;

    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer used for storing incoming data
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }


    // Initialize serverAddress structure used by bind
    sockaddr_in serverAddress;
    memset((char*)&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;				// IPv4 address family
    serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
    serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port


    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address family
        SOCK_STREAM,  // Stream socket
        IPPROTO_TCP); // TCP protocol

// Check if socket is successfully created
    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address to socket
    iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Check if socket is successfully binded to address and port from sockaddr_in structure
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("CLIENT listener socket is set to listening mode.\n");
    clientThreadStruct cli;

    unsigned long nb_listen = 1;
    ioctlsocket(listenSocket, FIONBIO, &nb_listen);

    do
    {
        if (WaitForSingleObject(semaphoreEnd, 10) == WAIT_OBJECT_0)
            break;
        // Struct for information about connected client
        sockaddr_in clientAddr;

        int clientAddrSize = sizeof(struct sockaddr_in);

        // Accept new connections from clients 
        acceptedSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (acceptedSocket != SOCKET_ERROR) {

            printf("\nNew client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            //create a new thread for a new client connected
            HANDLE hClient;
            DWORD ClientID;

            cli.acceptedSocket = acceptedSocket;
            sprintf(cli.clientName, "Client%d", client_count);


            hClient = CreateThread(NULL, 0, &client_read_write, (LPVOID)&cli, 0, &ClientID);

            //add it to the hash table
            client_thread* newCli = (client_thread*)malloc(sizeof(client_thread));
            sprintf(newCli->clientName, "Client%d", client_count);
            newCli->clientThread = hClient;
            newCli->finished = false;
            newCli->acceptedSocket = acceptedSocket;

            insert_client(newCli);
        }
        else
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                // U pitanju je blokirajuca operacija koja zbog rezima
                // soketa nece biti izvrsena 
                continue;
            }
            else {
                printf("[WORKER]: accept failed with error: %d\n", WSAGetLastError());
                break;
                // Desila se neka druga greska prilikom poziva operacije
            }
        }


    } while (true);

    //Close listen and accepted sockets
    closesocket(listenSocket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}