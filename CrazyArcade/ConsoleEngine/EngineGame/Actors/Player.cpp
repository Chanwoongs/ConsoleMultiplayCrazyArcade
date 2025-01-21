#include "PrecompiledHeader.h"

#include "Player.h"

#include "Engine/Engine.h"
#include "EngineGame/Levels/GameLevel.h"

Player::Player(const Vector2& position, GameLevel* level)
    : DrawableActor("P"), refLevel(level)
{
    this->position = position;

    color = Color::White;
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
