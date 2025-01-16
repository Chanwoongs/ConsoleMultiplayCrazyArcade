#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#define BUF_SIZE  1024
#define RLT_SIZE 4
#define OPSZ 4
#pragma comment (lib, "Ws2_32.lib")

void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;

    char opmsg[BUF_SIZE];
    int result, opndCnt, i;

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

    fputs("Operand count: ", stdout);
    scanf_s("%d", &opndCnt);
    opmsg[0] = (char)opndCnt;

    for (int i = 0; i < opndCnt; ++i)
    {
        printf("Operand %d: ", i + 1);
        scanf_s("%d", (int*)&opmsg[i * OPSZ + 1]);
    }
    fgetc(stdin);
    fputs("Operator: ", stdout);
    scanf_s("%c", &opmsg[opndCnt * OPSZ + 1], 1);
    send(hSocket, opmsg, opndCnt * OPSZ + 1 + 1, 0);

    recv(hSocket, reinterpret_cast<char*>(&result), RLT_SIZE, 0);

    printf("Operation result: %d \n", result);

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