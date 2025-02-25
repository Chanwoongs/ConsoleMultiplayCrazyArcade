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

void Vector2::Serialize(char* buffer, size_t& size)
{
    memcpy(buffer + size, &x, sizeof(uint32_t));
    size += sizeof(uint32_t);

    memcpy(buffer + size, &y, sizeof(uint32_t));
    size += sizeof(uint32_t);
}

void Vector2::Deserialize(const char* buffer, size_t& size)
{
    memcpy(&x, buffer + size, sizeof(uint32_t));
    size += sizeof(uint32_t);

    memcpy(&y, buffer + size, sizeof(uint32_t));
    size += sizeof(uint32_t);
}