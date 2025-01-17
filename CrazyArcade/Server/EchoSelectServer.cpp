#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment (lib, "ws2_32.lib")

#define TEST 1

#define BUF_SIZE 1024
void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;
    int clntAddrSize;

    char buf[BUF_SIZE];
    TIMEVAL timeout;
    fd_set reads, cpyReads;
    int strLen, fdNum, i;

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
    
    FD_ZERO(&reads);
    FD_SET(hServSock, &reads);

    while (true)
    {
        // 원본 저장
        cpyReads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        // select에서 에러 반환
        if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
        {
            break;
        }
        
        // select에서 입력된 데이터가 없어 타임 아웃 발생
        if (fdNum == 0)
        {
            continue;
        }

        for (int i = 0; i < reads.fd_count; ++i)
        {
            // 상태 변화가 있었던 소켓 검색
            if (FD_ISSET(reads.fd_array[i], &cpyReads))
            {
                // 상태 변화가 맞으면 accept로 연결 요청에 대한 수락 과정 진행
                if (reads.fd_array[i] == hServSock) // connection request!
                {
                    clntAddrSize = sizeof(clntAddr);
                    hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
                    // 클라이언트와 연결된 파일 디스크립터 정보를 등록
                    FD_SET(hClntSock, &reads);
                    printf("connected client: %d \n", (int)hClntSock);
                }
                else // Read Message
                {
                    strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
                    if (strLen == 0) // close request
                    {
                        // 읽은 idx의 값을 0으로 초기화
                        FD_CLR(reads.fd_array[i], &reads);
                        closesocket(cpyReads.fd_array[i]);
                        printf("closed client: %d \n", (int)cpyReads.fd_array[i]);
                    }
                    else
                    {
                        buf[strLen] = '\0';

                        char sendMsg[BUF_SIZE];
                        sprintf_s(sendMsg, BUF_SIZE, "%s", buf);

                        for (int j = 0; j < reads.fd_count; j++)
                        {
                            SOCKET targetSock = reads.fd_array[j];

                            if (targetSock != hServSock)
                            {
                                send(targetSock, sendMsg, (int)strlen(sendMsg), 0);
                            }
                        }
                    }
                }
            }
        }
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