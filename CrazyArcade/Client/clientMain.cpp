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

    //engine->Run();

    //while (!client->IsGameover())
    //{
        //if (Engine::Get().GetKeyDown(VK_RETURN))
        //{
            InputPacket* inputPacket = new InputPacket(PacketType::INPUT, sizeof(InputPacket), 1, VK_RETURN);
            PacketType* type = new PacketType;
            *type = PacketType::INPUT;
            PacketData* packetData = new PacketData(client , type, (void*)inputPacket);
            client->RunSendThread(packetData);
        //}
    //}

    std::cin.get();
    
    delete engine;
    delete client;

    return 0;
}