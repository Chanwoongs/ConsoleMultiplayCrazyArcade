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

class GameClient
{
public:
	GameClient(const char* ip, const char* port);
	~GameClient();

	void ConnectServer();
	void RunThreads();

	static unsigned WINAPI Send(void* arg);
	static unsigned WINAPI Receive(void* arg);

	void ErrorHandling(const char* message);

private:
	SOCKET hSocket;
	SOCKADDR_IN serverAddress;

	HANDLE hSendThread, hReceiveThread;
};