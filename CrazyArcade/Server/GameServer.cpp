#include "GameServer.h"

struct ClientHandleData 
{
    GameServer* server; 
    SOCKET hClientSocket;
};

GameServer::GameServer(const char* port)
    : port(atoi(port)), clientCount(0)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hServerSocket == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

#if TEST
    serverAddress.sin_port = htons(9190);
#else
    serverAddress.sin_port = htons(atoi(port));
#endif

    if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        ErrorHandling("bind() error");
    }

    if (listen(hServerSocket, 5) == SOCKET_ERROR)
    {
        ErrorHandling("listen() error");
    }
}

GameServer::~GameServer()
{
    closesocket(hServerSocket);
    WSACleanup();

    CloseHandle(hThread);
    CloseHandle(hMutex);
}

void GameServer::AcceptClients()
{
    while (true)
    {
        SOCKADDR_IN clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);

        WaitForSingleObject(hMutex, INFINITE);
        clientSockets.push_back(clientSocket);
        ++clientCount;
        ReleaseMutex(hMutex);

        ClientHandleData* clientHandleData = new ClientHandleData{ this, clientSocket };
        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)clientHandleData, 0, NULL);

        char clientIP[20] = { 0 };
        if (inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP)))
        {
            printf("Connected client IP: %s \n", clientIP);
        }
        else
        {
            ErrorHandling("inet_ntop() error");
        }
    }
}

unsigned WINAPI GameServer::HandleClient(void* arg)
{
    ClientHandleData* clientHandleData = static_cast<ClientHandleData*>(arg);
    GameServer* server = clientHandleData->server;
    SOCKET hClientSocket = clientHandleData->hClientSocket;

    int strLen = 0;
    char msg[BUF_SIZE];

    while ((strLen = recv(hClientSocket, msg, sizeof(msg), 0)) > 0)
    {
        server->Broadcast(msg, strLen);
    }

    WaitForSingleObject(server->hMutex, INFINITE);
    auto it = std::find(server->clientSockets.begin(), server->clientSockets.end(), 
        hClientSocket);
    if (it != server->clientSockets.end())
    {
        server->clientSockets.erase(it);
        --server->clientCount;
    }

    ReleaseMutex(server->hMutex);
    closesocket(hClientSocket);

    delete clientHandleData;
    return 0;
}

void GameServer::Broadcast(char* msg, int len)
{
    WaitForSingleObject(hMutex, INFINITE);
    for (auto& client : clientSockets)
    {
        send(client, msg, len, 0);
    }
    ReleaseMutex(hMutex);
}

void GameServer::ErrorHandling(const char* message) const
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}