#include "PrecompiledHeader.h"

#include "Ground.h"

Ground::Ground(const Vector2& position)
    : DrawableActor(position, " ")
{
    this->position = position;

    color = Color::Red;
}
