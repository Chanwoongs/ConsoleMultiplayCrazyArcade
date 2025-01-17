#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

#define BUF_SIZE  100
#define NAME_SIZE 20

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

#define Test 1

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);
void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;
    HANDLE hSndThread, hRcvThread;

#if !Test
    if (argc != 4)
    {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        exit(0);
    }
#endif

    // 소켓 라이브러리 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    // 소켓 생성, TCP 소켓 생성
    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hSocket == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &servAddr.sin_addr.s_addr);
    servAddr.sin_port = htons(atoi(argv[2]));

    // 생성된 소켓을 바탕으로 서버에 연결 요청
    if (connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        ErrorHandling("connect() error");
    }
    else
    {
        puts("Connected.........");
    }

    hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSocket, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSocket, 0, NULL);

    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);

    closesocket(hSocket);

    // 소켓 라이브러리 해제
    WSACleanup();

    return 0;
}

unsigned WINAPI SendMsg(void* arg) // send thread main
{
    SOCKET hSocket = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE + BUF_SIZE];
    while (true)
    {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            closesocket(hSocket);
            exit(0);
        }
        sprintf_s(nameMsg, sizeof(nameMsg), "%s %s", name, msg);
        send(hSocket, nameMsg, strlen(nameMsg), 0);
    }

    return 0;
}

unsigned WINAPI RecvMsg(void* arg) // read thread main
{
    SOCKET hSocket = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE + BUF_SIZE];
    int strLen;
    while (true)
    {
        strLen = recv(hSocket, nameMsg, NAME_SIZE + BUF_SIZE - 1, 0);
        if (strLen == -1)
        {
            return -1;
        }
        nameMsg[strLen] = 0;
        fputs(nameMsg, stdout);
    }

    return 0;
}

void ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}