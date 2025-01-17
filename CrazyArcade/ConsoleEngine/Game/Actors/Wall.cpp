#include "Wall.h"

Wall::Wall(const Vector2& position)
    : DrawableActor("#")
{
    // Set Position
    this->position = position;

    // Set Color
    color = Color::Green;
}