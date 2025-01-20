#include "GameClient.h"

GameClient::GameClient(const char* ip, const char* port)
    : isGameover(false)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hSocket == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serverAddress.sin_addr.s_addr);
    serverAddress.sin_port = htons(atoi(port));

    ConnectServer();

    //@TODO: 서버에 연결된 뒤에 서버에게 플레이어 ID를 받아야함.
}

GameClient::~GameClient()
{
    closesocket(hSocket);

    WSACleanup();

    CloseHandle(hSendMutex);
    CloseHandle(hSendThread);
    CloseHandle(hReceiveThread);
}

void GameClient::ConnectServer()
{
    if (connect(hSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        ErrorHandling("connect() error");
    }
    else
    {
        puts("Connected.........");
    }
}

void GameClient::RunThreads()
{
    hSendThread = (HANDLE)_beginthreadex(NULL, 0, Send, (void*)&hSocket, 0, NULL);
    hReceiveThread = (HANDLE)_beginthreadex(NULL, 0, Receive, (void*)&hSocket, 0, NULL);

    WaitForSingleObject(hSendThread, INFINITE);
    WaitForSingleObject(hReceiveThread, INFINITE);
}

void GameClient::RunSendThread(void* arg)
{
    hSendThread = (HANDLE)_beginthreadex(NULL, 0, Send, arg, 0, NULL);
}

unsigned WINAPI GameClient::Send(void* arg)
{
    GameClient* client = static_cast<GameClient*>(arg);

    while (!client->IsGameover())
    {
        PacketData* packetData = nullptr;

        WaitForSingleObject(client->hSendMutex, INFINITE);
        if (!client->sendQueue.empty())
        {
            packetData = client->sendQueue.front();
            client->sendQueue.pop();
        }
        ReleaseMutex(client->hSendMutex);

        if (packetData)
        {
            SOCKET hSocket = client->Socket();
            PacketType* type = packetData->packetType;
            void* packet = packetData->packet;

            if (*type == PacketType::INPUT)
            {
                char buffer[MAX_BUFFER_SIZE] = {};
                SerializePacket(packet, sizeof(buffer), buffer);
                send(hSocket, buffer, MAX_BUFFER_SIZE, 0);
            }

            delete packet;
            delete type;
            delete packetData;
        }
        else
        {
            Sleep(10);
        }
    }

    return 0;
}


unsigned WINAPI GameClient::Receive(void* arg)
{
    SOCKET hSocket = *((SOCKET*)arg);
    char message[NAME_SIZE + BUF_SIZE] = {};
    int strLen;
    while (true)
    {
        strLen = recv(hSocket, message, NAME_SIZE + BUF_SIZE - 1, 0);
        if (strLen == -1)
        {
            return -1;
        }
        message[strLen] = 0;
        fputs("message from server: ", stdout);
        fputs(message, stdout);
    }

    return 0;
}

void GameClient::AddPacketToSendQueue(PacketData* data)
{
    WaitForSingleObject(hSendMutex, INFINITE);
    sendQueue.push(data);
    ReleaseMutex(hSendMutex);
}

void GameClient::ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}