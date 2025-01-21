#include "PrecompiledHeader.h"

#include "Actor.h"

Actor::Actor()
	: isActive(true), isExpired(false)
{
}

Actor::~Actor()
{
}

void Actor::Update(float deltaTime)
{
}

void Actor::Draw()
{
}

void Actor::Serialize(const void* data, size_t dataSize, char* buffer)
{
    memcpy(buffer, data, dataSize);
}

void Actor::SetPosition(const Vector2& newPosition)
{
    position = newPosition;
}

inline Vector2 Actor::Position() const
{
    return position;
}
