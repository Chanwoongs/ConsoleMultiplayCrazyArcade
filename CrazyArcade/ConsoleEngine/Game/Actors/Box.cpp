#include "Box.h"

Box::Box(const Vector2& position)
    : DrawableActor("b")
{
    this->position = position;

    color = Color::White;
}
