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

void Actor::Serialize(char* buffer, size_t& size)
{
    size_t offset = size;

    size_t positionSize = 0;
    position.Serialize(buffer + offset, positionSize);
    offset += positionSize;

    size += offset;
}

void Actor::Deserialize(const char* buffer, size_t& size)
{
    size_t offset = size;

    size_t positionSize = 0;
    position.Deserialize(buffer + offset, positionSize);
    offset += positionSize;

    size += offset;
}

void Actor::SetPosition(const Vector2& newPosition)
{
    position = newPosition;
}

inline Vector2 Actor::Position() const
{
    return position;
}
