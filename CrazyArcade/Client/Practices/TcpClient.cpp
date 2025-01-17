#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;

    char message[30];
    int strLen = 0;
    int idx = 0, readLen = 0;

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

    // while문 안에서 recv 함수를 통해, 수신된 데이터를 1바이트씩 읽는다
    while (readLen = recv(hSocket, &message[idx++], 1, 0))
    {
        if (readLen == -1)
        {
            ErrorHandling("read() error");
        }

        // 1바이트씩 데이터를 읽고 있기 때문에 strLen에 1씩 더해지며, 이는 recv 함수의 호출 횟 수와 같다.
        strLen += readLen;
    }

    printf("Message from server: %s \n", message);
    printf("Function read call count: %d \n", strLen);

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