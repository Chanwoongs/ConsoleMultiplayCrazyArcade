#include "PrecompiledHeader.h"

#include "Player.h"

#include "Engine/Engine.h"
#include "EngineGame/Levels/GameLevel.h"

Player::Player(int id, const Vector2& position, GameLevel* level)
    : DrawableActor(position, "P"), id(id), refLevel(level)
{
    this->position = position;

    color = Color::White;
}

void Player::Serialize(char* buffer, size_t& size)
{
    Super::Serialize(buffer, size);

    size_t offset = 0;

    memcpy(buffer + offset, &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    size += offset;
}

void Player::Deserialize(const char* buffer, size_t& size)
{
    Super::Deserialize(buffer, size);

    size_t offset = 0;

    memcpy(&id, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    size += offset;
}

void Player::Update(float deltaTime)
{
    Super::Update(deltaTime);

    if (Engine::Get().GetKeyDown(VK_ESCAPE))
    {

    }

    // 상하좌우
    if (Engine::Get().GetKeyDown(VK_LEFT))
    {
        if (refLevel->CanPlayerMove(Vector2(position.x - 1, position.y)))
        {
            position.x -= 1;
        }
    }   
    if (Engine::Get().GetKeyDown(VK_RIGHT))
    {
        if (refLevel->CanPlayerMove(Vector2(position.x + 1, position.y)))
        {
            position.x += 1;
        }
    }
    if (Engine::Get().GetKeyDown(VK_UP))
    {
        if (refLevel->CanPlayerMove(Vector2(position.x, position.y - 1)))
        {
            position.y -= 1;
        }
    }
    if (Engine::Get().GetKeyDown(VK_DOWN))
    {
        if (refLevel->CanPlayerMove(Vector2(position.x, position.y + 1)))
        {
            position.y += 1;
        }
    }
}
