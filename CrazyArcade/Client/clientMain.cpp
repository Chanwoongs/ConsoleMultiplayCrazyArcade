#include <iostream>

#include "GameClient.h"

#include "Engine/Engine.h"
#include "Network/Packets.h"

#define TEST 1

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#if !TEST
    if (argc != 4)
    {
        printf("Usage: %s <IP> <PORT> <USERNAME>\n", argv[0]);
        exit(0);
    }
    GameClient* client = new GameClient(argv[2], argv[3]);
#else
    GameClient* client = new GameClient("127.0.0.1", "9190");
#endif
    Engine* engine = new Engine();

    client->RunThreads();

    bool hasEnteredGame = false;
    while (true)
    {
        engine->ProcessInput();
        if (engine->GetKeyDown(VK_RETURN))
        {
            //InputPacket* inputPacket = new InputPacket(1, VK_RETURN);
            //PacketData* packetData = new PacketData(client, PacketType(inputPacket->header.packetType), (void*)inputPacket);
            //client->AddPacketToSendQueue(packetData);

            if (!hasEnteredGame)
            {
                PlayerEnterRequestPacket* playerEnterRequestPacket = new PlayerEnterRequestPacket;
                PacketData* playerEnterRequestPacketData =
                    new PacketData(client, PacketType(playerEnterRequestPacket->header.packetType), (void*)playerEnterRequestPacket);
                client->EnqueueSend(playerEnterRequestPacketData);
                hasEnteredGame = true;
            }
        }
        engine->SavePreviousKeyStates();
    }

    delete engine;
    delete client;

    return 0;
}