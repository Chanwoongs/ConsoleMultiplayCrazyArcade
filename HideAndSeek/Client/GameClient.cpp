#include "GameClient.h"

#include "ClientGame/Game.h"
#include "Network/Packets.h"

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
    hReceiveMutex = CreateMutex(NULL, FALSE, NULL);
    if (hReceiveMutex == NULL)
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

    if (serverAddress)
    {
        delete serverAddress;
    }

    WaitForSingleObject(hSendThread, INFINITE);
    WaitForSingleObject(hReceiveThread, INFINITE);

    CloseHandle(hSendThread);
    CloseHandle(hReceiveThread);
}

ClientPacketData* GameClient::CreatePacketData(const PacketType packetType, const size_t packetSize, char* packet)
{
    return new ClientPacketData(this, packetType, packetSize, packet);
}

void GameClient::ConnectServer()
{
    if (connect(hSocket, (SOCKADDR*)serverAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        int errorCode = WSAGetLastError();
        printf("Connect Error: %d\n", errorCode);

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
        ClientPacketData* packetData = nullptr;

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
            send(hSocket, buffer, (int)packetData->size, 0);

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

void GameClient::EnqueueSend(ClientPacketData* data)
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
        Game::Get().RequestLevelChange(new GameLevel);

        PlayerCreateRequestPacket* playerCreateRequestPacket = new PlayerCreateRequestPacket();

        EnqueueSend(
            CreatePacketData(
                PacketType::PLAYER_CREATE_REQUEST,
                sizeof(playerCreateRequestPacket),
                (char*)playerCreateRequestPacket)
        );

        delete playerCreateRequestPacket;
    }
    else if ((PacketType)packetHeader->packetType == PacketType::PLAYER_CREATE_RESPOND)
    {
        PlayerCreateRespondPacket playerCreateRespondPacket(0);
        playerCreateRespondPacket.Deserialize(packet, size);

        playerId = playerCreateRespondPacket.playerId;

        printf("부여 받은 PlayerId : %d", playerId);

        WaitForSingleObject(hReceiveMutex, INFINITE);
        
        GameLevel* currentLevel = static_cast<GameLevel*>(Game::Get().GetCurrentLevel());
		currentLevel->SetClientId(playerId);

        ReleaseMutex(hReceiveMutex);

        Game::Get().EnterGame();
    }
    else if ((PacketType)packetHeader->packetType == PacketType::MOVE_PATH)
    {
        WaitForSingleObject(hReceiveMutex, INFINITE);

        MovePathPacket* movePathPacket = new MovePathPacket(0, 0, nullptr);
        movePathPacket->Deserialize(packet, size);

        std::vector<Vector2*> path;
        size_t offset = 0;
        while (offset < movePathPacket->pathBufferSize)
        {
            Vector2* position = new Vector2(0, 0);
            position->Deserialize(movePathPacket->pathBuffer, offset);
            path.push_back(position);
        }

        GameLevel* currentLevel = static_cast<GameLevel*>(Game::Get().GetCurrentLevel());
        currentLevel->RequestClientPlayerPathChange(std::move(path));

        delete movePathPacket;

        ReleaseMutex(hReceiveMutex);
    }
    else if ((PacketType)packetHeader->packetType == PacketType::GAME_STATE_SYNCHRONIZE)
    {
        if (!hasEnteredGame) return;

        WaitForSingleObject(hReceiveMutex, INFINITE);
        GameStateSynchronizePacket gameStateSynchronizePacket(0, nullptr);
        gameStateSynchronizePacket.Deserialize(packet, size);

        GameLevel* currentLevel = static_cast<GameLevel*>(Game::Get().GetCurrentLevel());
        currentLevel->DeserializeGameState(gameStateSynchronizePacket.gameStateBuffer);
        ReleaseMutex(hReceiveMutex);
    }
}

void GameClient::RequestExitGame()
{
    PlayerExitRequestPacket* playerExitequestPacket = new PlayerExitRequestPacket(playerId);

    GameClient::Get().EnqueueSend(
        GameClient::Get().CreatePacketData(
            PacketType(playerExitequestPacket->header.packetType),
            sizeof(PlayerExitRequestPacket),
            (char*)playerExitequestPacket)
    );

    delete playerExitequestPacket;
}

void GameClient::ErrorHandling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}