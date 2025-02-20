#include "Game.h"

#include "GameClient.h"
#include "Levels/MenuLevel.h"
#include "EngineGame/Levels/GameLevel.h"

Game* Game::instance = nullptr;

Game::Game()
    : Engine()
{
    instance = this;

    menuLevel = new MenuLevel();
}

Game::~Game()
{
    if (showMenu)
    {
        delete backLevel;
        backLevel = nullptr;
        mainLevel = nullptr;
    }
    else
    {
        delete mainLevel;
        mainLevel = nullptr;
    }

    delete menuLevel;
    menuLevel = nullptr;
}

void Game::ToggleMenu()
{
    system("cls");

    showMenu = !showMenu;
    if (showMenu)
    {
        backLevel = mainLevel;
        mainLevel = menuLevel;
    }
    else
    {
        mainLevel = backLevel;
    }
}

void Game::RequestEnterGame()
{
    PlayerEnterRequestPacket* playerEnterRequestPacket = new PlayerEnterRequestPacket;

    GameClient::Get().EnqueueSend(
        GameClient::Get().CreatePacketData(
            PacketType(playerEnterRequestPacket->header.packetType),
            sizeof(PlayerEnterRequestPacket),
            (char*)playerEnterRequestPacket)
    );

    delete playerEnterRequestPacket;
}

void Game::EnterGame()
{
    system("cls");

    GameClient::Get().EnterGame();
}

void Game::CheckInput()
{
    if (!GameClient::Get().HasEnteredGame()) return;

    if (Engine::Get().GetKeyDown(VK_UP))
    {
        InputPacket* inputPacket = new InputPacket(GameClient::Get().PlayerId(), VK_UP);
        
        size_t serializedSize = 0;
        char* serializedData = inputPacket->Serialize(serializedSize);
        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(inputPacket->header.packetType),
                serializedSize,
                serializedData));
        //InputPacket* i = new InputPacket(0, 0);
        //size_t s = 0;
        //i->Deserialize(serializedData, s);

        delete inputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_DOWN))
    {
        InputPacket* inputPacket = new InputPacket(GameClient::Get().PlayerId(), VK_DOWN);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(inputPacket->header.packetType),
                sizeof(InputPacket),
                (char*)inputPacket));

        delete inputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_RIGHT))
    {
        InputPacket* inputPacket = new InputPacket(GameClient::Get().PlayerId(), VK_RIGHT);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(inputPacket->header.packetType),
                sizeof(InputPacket),
                (char*)inputPacket));

        delete inputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_LEFT))
    {
        InputPacket* inputPacket = new InputPacket(GameClient::Get().PlayerId(), VK_LEFT);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(inputPacket->header.packetType),
                sizeof(InputPacket),
                (char*)inputPacket));

        delete inputPacket;
    }
}
