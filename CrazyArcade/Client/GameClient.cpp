#include "GameClient.h"

#include "ClientGame/Game.h"

GameClient* GameClient::Instance = nullptr;

GameClient::GameClient(const char* ip, const char* port)
{
    serverAddress = new SOCKADDR_IN();

    Instance = this;

    WSADATA wsaData;

    hSendMutex = CreateMutex(NULL, FALSE, NULL);
    if (hSendMutex == NULL)
    {
        ErrorHandling("Mutex creation error!");
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        ErrorHandling("WSAStartup() error!");
    }

    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hSocket == INVALID_SOCKET)
    {
        ErrorHandling("socket() error");
    }

    memset(serverAddress, 0, sizeof(SOCKADDR_IN));
    serverAddress->sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(serverAddress->sin_addr.s_addr));
    serverAddress->sin_port = htons(atoi(port));

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

    if (serverAddress)
    {
        delete serverAddress;
    }
}

GameClient::PacketData* GameClient::CreatePacketData(PacketType packetType, size_t packetSize, char* packet)
{
    return new GameClient::PacketData(this, packetType, packetSize, packet);
}

void GameClient::ConnectServer()
{
    if (connect(hSocket, (SOCKADDR*)serverAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
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
    hSendThread = (HANDLE)_beginthreadex(NULL, 0, Send, this, 0, NULL);
    if (hSendThread == NULL)
    {
        ErrorHandling("Send thread creation error!");
    }

    hReceiveThread = (HANDLE)_beginthreadex(NULL, 0, Receive, this, 0, NULL);
    if (hReceiveThread == NULL)
    {
        ErrorHandling("Receive thread creation error!");
    }
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
            PacketType type = packetData->type;
            char* packet = packetData->packet;
            
            char buffer[packetBufferSize] = {};
            SerializePacket(packet, packetData->size, buffer);
            send(hSocket, buffer, packetData->size, 0);

            delete packet;
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
    GameClient* client = static_cast<GameClient*>(arg);
    SOCKET hSocket = client->Socket();
    char buffer[packetBufferSize] = {};
    int strLen;

    while (!client->IsGameover())
    {
        strLen = recv(hSocket, buffer, packetBufferSize, 0);

        if (strLen > 0)
        {
            client->ProcessPacket(buffer, strLen);
        }
    }
    return 0;
}

void GameClient::EnqueueSend(PacketData* data)
{
    WaitForSingleObject(hSendMutex, INFINITE);
    sendQueue.push(data);
    ReleaseMutex(hSendMutex);
}

void GameClient::ProcessPacket(char* packet, int size)
{
    PacketHeader* packetHeader = (PacketHeader*)packet;

    if ((PacketType)packetHeader->packetType == PacketType::PLAYER_ENTER_RESPOND)
    {
        PlayerEnterRespondPacket playerEnterRespondPacket(0, 0, 0, nullptr, 0);
        playerEnterRespondPacket.Deserialize(packet, size);

        playerId = playerEnterRespondPacket.playerId;

        printf("부여 받은 PlayerId : %d", playerId);

        Game::Get().LoadLevel(new GameLevel);

        GameLevel* currentLevel = static_cast<GameLevel*>(Game::Get().GetCurrentLevel());
        currentLevel->DeserializeGameState(playerEnterRespondPacket.gameStateBuffer);

        Game::Get().EnterGame();
    }
    else if ((PacketType)packetHeader->packetType == PacketType::GAME_STATE_SYNCHRONIZE)
    {
        if (!hasEnteredGame) return;

        GameStateSynchronizePacket gameStateSynchronizePacket(0, nullptr);
        gameStateSynchronizePacket.Deserialize(packet, size);

        GameLevel* currentLevel = static_cast<GameLevel*>(Game::Get().GetCurrentLevel());
        currentLevel->DeserializeGameState(gameStateSynchronizePacket.gameStateBuffer);
    }
}

void GameClient::ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}