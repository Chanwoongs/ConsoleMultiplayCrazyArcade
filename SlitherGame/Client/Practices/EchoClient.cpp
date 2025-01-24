#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#define BUF_SIZE  1024
#pragma comment (lib, "Ws2_32.lib")

void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;

    char message[BUF_SIZE];
    int strLen = 0;

    if (argc != 3)
    {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        exit(0);
    }

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

    // while문 안에서 recv 함수를 통해, 수신된 데이터를 1바이트씩 읽는다
    while (true)
    {
        fputs("Input Message(Q to quit): ", stdout);
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }

        send(hSocket, message, strlen(message), 0);
        strLen = recv(hSocket, message, (int)(BUF_SIZE - 1), 0);
        message[strLen] = '\0';
        printf("Message from server: %s", message);
    }

    closesocket(hSocket);

    // 소켓 라이브러리 해제
    WSACleanup();

    return 0;
}

void ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}