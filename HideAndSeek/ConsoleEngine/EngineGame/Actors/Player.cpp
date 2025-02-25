#include "PrecompiledHeader.h"

#include "Player.h"

#include "Engine/Engine.h"
#include "EngineGame/Levels/GameLevel.h"

Player::Player(int id, const Vector2& position, GameLevel* level)
    : DrawableActor(position, "H"), id(id), refLevel(level), isSeeker(false)
{
    this->position = position;

    color = Color::Blue;
}

void Player::Serialize(char* buffer, size_t& size)
{
    Super::Serialize(buffer, size);

    size_t offset = size;

    memcpy(buffer + offset, &id, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer + offset, &isSeeker, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    size += offset;
}

void Player::Deserialize(const char* buffer, size_t& size)
{
    Super::Deserialize(buffer, size);

    size_t offset = size;

    memcpy(&id, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&isSeeker, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    size += offset;
}

void Player::Update(float deltaTime)
{
    Super::Update(deltaTime);
}

void Player::Draw()
{
    if (isSeeker)
    {
        Engine::Get().Draw(position, image, Color::Red);
    }
    else
    {
        Engine::Get().Draw(position, image, Color::Blue);
    }
}

void Player::Initialize(const int clientId)
{
    isSeeker ? SetImage("S") : SetImage("H");

    if (clientId == id)
    {
        SetImage("P");
    }
}
