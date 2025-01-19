#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <WS2tcpip.h>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#define BUF_SIZE 256

class GameServer
{
public:
	GameServer(char* port);
	~GameServer();

	void AcceptClients();
	static unsigned WINAPI HandleClient(void* arg);
	void Broadcast(char* msg, int len);


	void ErrorHandling(const char* message) const;

private:
	SOCKET hServerSocket;
	std::vector<SOCKET> clientSockets;

	SOCKADDR_IN serverAddress;

	HANDLE hMutex;

	int port;
	int clientCount;
};