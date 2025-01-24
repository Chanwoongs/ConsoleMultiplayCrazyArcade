#include "PrecompiledHeader.h"

#include "Wall.h"

Wall::Wall(const Vector2& position)
    : DrawableActor(position, "#")
{
    // Set Position
    this->position = position;

    // Set Color
    color = Color::Green;
}