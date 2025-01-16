#include "Target.h"

Target::Target(const Vector2& position)
    : DrawableActor("T")
{
    this->position = position;

    color = Color::Blue;
}
