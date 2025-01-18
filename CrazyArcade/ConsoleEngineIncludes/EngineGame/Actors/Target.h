#pragma once

#include "Actor/DrawableActor.h"

class ENGINE_API Target : public DrawableActor
{
    RTTI_DECLARATIONS(Target, DrawableActor)

public:
    Target(const Vector2& position);
};