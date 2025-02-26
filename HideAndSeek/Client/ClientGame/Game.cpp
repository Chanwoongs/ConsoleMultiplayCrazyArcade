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
    GameClient::Get().EnterGame();
}

void Game::CheckInput()
{
    if (!GameClient::Get().HasEnteredGame()) return;

    Engine::CheckInput();

    if (Engine::Get().GetKeyDown(VK_UP))
    {
        KeyInputPacket* keyInputPacket = new KeyInputPacket(GameClient::Get().PlayerId(), VK_UP);
        
        size_t serializedSize = 0;
        char* serializedData = keyInputPacket->Serialize(serializedSize);
        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(keyInputPacket->header.packetType),
                serializedSize,
                serializedData));
        //InputPacket* i = new InputPacket(0, 0);
        //size_t s = 0;
        //i->Deserialize(serializedData, s);

        delete keyInputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_DOWN))
    {
        KeyInputPacket* keyInputPacket = new KeyInputPacket(GameClient::Get().PlayerId(), VK_DOWN);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(keyInputPacket->header.packetType),
                sizeof(KeyInputPacket),
                (char*)keyInputPacket));

        delete keyInputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_RIGHT))
    {
        KeyInputPacket* keyInputPacket = new KeyInputPacket(GameClient::Get().PlayerId(), VK_RIGHT);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(keyInputPacket->header.packetType),
                sizeof(KeyInputPacket),
                (char*)keyInputPacket));

        delete keyInputPacket;
    }
    if (Engine::Get().GetKeyDown(VK_LEFT))
    {
        KeyInputPacket* keyInputPacket = new KeyInputPacket(GameClient::Get().PlayerId(), VK_LEFT);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(keyInputPacket->header.packetType),
                sizeof(KeyInputPacket),
                (char*)keyInputPacket));

        delete keyInputPacket;
    }
    
    if (Engine::Get().GetKeyDown(VK_LBUTTON))
    {
        MouseInputPacket* mouseInputPacket = new MouseInputPacket(GameClient::Get().PlayerId(), VK_LBUTTON, Engine::Get().MousePosition().y, Engine::Get().MousePosition().x);

        GameClient::Get().EnqueueSend(
            GameClient::Get().CreatePacketData(
                PacketType(mouseInputPacket->header.packetType),
                sizeof(MouseInputPacket),
                (char*)mouseInputPacket));

        delete mouseInputPacket;
    }
}
