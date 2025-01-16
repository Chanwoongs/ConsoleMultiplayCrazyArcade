#include "PrecompiledHeader.h"
#include "Vector2.h"

Vector2::Vector2(int x, int y)
    : x(x), y(y)
{
}

Vector2 Vector2::operator+(const Vector2& other)
{
    return Vector2(this->x + other.x, this->y + other.y);
}

Vector2 Vector2::operator-(const Vector2& other)
{
    return Vector2(this->x - other.x, this->y - other.y);
}

bool Vector2::operator==(const Vector2& other)
{
    return this->x == other.x && this->y == other.y;
}

bool Vector2::operator!=(const Vector2& other)
{
    return this->x != other.x || this->y != other.y;
    // return !(*this == other);
}