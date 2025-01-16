#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib")

#define BUF_SIZE 1024
void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    char message[BUF_SIZE];
    int strLen, i;

    SOCKADDR_IN servAddr, clntAddr;
    int clntAddrSize;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n ", argv[0]);
        exit(1);
    }

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
    servAddr.sin_port = htons(atoi(argv[1]));

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


    // 클라이언트의 연결 요청을 수락하기 위해서 accept 함수 호출
    clntAddrSize = sizeof(clntAddr);

    for (int i = 0; i < 5; i++)
    {
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
        if (hClntSock == INVALID_SOCKET)
        {
            ErrorHandling("accept() error");
        }
        else
        {
            printf("Connected client: %d \n", i + 1);
        }

        while ((strLen = recv(hClntSock, message, BUF_SIZE, 0)) != 0)
        {
            // send 함수 호출을 통해서 연결된 클라이언트에 데이터 전송
            send(hClntSock, message, strLen, 0);
        }
        closesocket(hClntSock);
    }

    closesocket(hServSock);

    // 초기화한 소켓 라이브러리 반환 해제 및 이후 왼속 관련 함수 호출 불가
    WSACleanup();

    return 0;
}

void ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}