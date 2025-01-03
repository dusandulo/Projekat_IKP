#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning(disable:4996)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 5059
#define BUFFER_SIZE 256

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


DWORD WINAPI client_read(LPVOID param) {
    SOCKET connectedSocket = (SOCKET)param;
    //check if we got data from client or EXIT signal
    //OR if we got a message from worker
    char dataBuffer[BUFFER_SIZE];
    do
    {
        while (!is_socket_ready(connectedSocket, true)) {

        }

        int iResult = recv(connectedSocket, dataBuffer, BUFFER_SIZE, 0);

        if (iResult != SOCKET_ERROR) {

            if (iResult > 0) {
                dataBuffer[iResult] = '\0';
                // Log message text
                printf("[CLIENT]: Worker/LB sent: %s.\n", dataBuffer);
            }
            else if (iResult == 0) {
                printf("[CLIENT]: Connection closed.\n");
                break;
            }

        }
        else	// There was an error during recv
        {

            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                continue;
            }
            else {
                printf("[CLIENT]: recv failed with error: %d\n", WSAGetLastError());
                break;
            }

        }

    } while (true);
}


// TCP client that use blocking sockets
int main()
{
    // Socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer we will use to store message
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;								// IPv4 protocol
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
    serverAddress.sin_port = htons(SERVER_PORT);					// server port

    // Connect to server specified in serverAddress and socket connectSocket
    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    u_long non_blocking = 1;
    ioctlsocket(connectSocket, FIONBIO, &non_blocking);

    HANDLE hClientListener;
    DWORD clientID;
    hClientListener = CreateThread(NULL, 0, &client_read, (LPVOID)connectSocket, 0, &clientID);
    int msgCnt = 0;

    while (msgCnt <= 1000) {
        // Sleep for 2 seconds
        Sleep(1000);

        // Generate a random number between 50 and 2000
        int randomNumber = rand() % (2000 - 50 + 1) + 50;

        // Format the random number as a string and copy it to the dataBuffer
        sprintf(dataBuffer, "%d", randomNumber);

        // Send the message to the server using the connected socket
        iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

        // Check if the message is "exit"
        if (strcmp(dataBuffer, "exit") == 0)
            break;

        msgCnt++;

        // Check the result of the send function
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        printf("Message successfully sent. Total bytes: %ld\n", iResult);
    }

    Sleep(3000);

    sprintf(dataBuffer, "exit");

    // Send message to server using connected socket
    iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);

    // For demonstration purpose
    printf("\nPress any key to exit: ");
    _getch();

    // Shutdown the connection since we're done
    iResult = shutdown(connectSocket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    // Close connected socket
    closesocket(connectSocket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}