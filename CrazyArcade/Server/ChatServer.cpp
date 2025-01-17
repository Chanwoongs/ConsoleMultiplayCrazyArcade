#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

#define TEST 1

#define BUF_SIZE 100
#define MAX_CLNT 256

unsigned WINAPI HandleClnt(void* arg);
void SendMsg(char* msg, int len);
void ErrorHandling(const char* message);

int clntCnt = 0;
SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex;

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;
    int clntAddrSize;
    HANDLE hThread;

#if !TEST
    if (argc != 2)
    {
        printf("Usage: %s <port>\n ", argv[0]);
        exit(1);
    }
#endif

    // 소켓 라이브러리 초기화 함수
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    // 소켓 생성
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

#if TEST
    servAddr.sin_port = htons(9190);
#else
    servAddr.sin_port = htons(atoi(argv[1]));
#endif


    // 소켓에 IP 주소와 PORT 번호 할당
    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
    {
        ErrorHandling("bind() error");
    }

    // listen 함수를 통해 생성한 소켓을 서버 소켓으로 완성
    if (listen(hServSock, 5) == SOCKET_ERROR)
    {
        ErrorHandling("listen() error");
    }

    while (true)
    {
        clntAddrSize = sizeof(clntAddr);
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);

        WaitForSingleObject(hMutex, INFINITE);
        clntSocks[clntCnt++] = hClntSock;
        ReleaseMutex(hMutex);

        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
        char clientIP[20] = { 0 };
        if (inet_ntop(AF_INET, &clntAddr.sin_addr, clientIP, sizeof(clientIP)))
        {
            printf("Connected client IP: %s \n", clientIP);
        }
        else
        {
            ErrorHandling("inet_ntop() error");
        }
    }
  
    closesocket(hServSock);

    // 초기화한 소켓 라이브러리 반환 해제 및 이후 왼속 관련 함수 호출 불가
    WSACleanup();

    return 0;
}

unsigned WINAPI HandleClnt(void* arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    while ((strLen = recv(hClntSock, msg, sizeof(msg), 0)) != 0)
    {
        SendMsg(msg, strLen);
    }

    WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < clntCnt; i++) // remove disconnected chilent
    {
        if (hClntSock == clntSocks[i])
        {
            while (i++ < clntCnt - 1)
            {
                clntSocks[i] = clntSocks[i + 1];

                break;
            }
        }
    }


    clntCnt--;
    ReleaseMutex(hMutex);
    closesocket(hClntSock);
    return 0;
}

void SendMsg(char* msg, int len) // send to all
{
    WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < clntCnt; i++)
    {
        send(clntSocks[i], msg, len, 0);
    }
    ReleaseMutex(hMutex);
}

void ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}