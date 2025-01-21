#pragma once

#include "Actor.h"

class ENGINE_API DrawableActor : public Actor
{
public:
    struct SerializeData
    {
        uint32_t width;
        uint32_t color;
        uint32_t imageLength;
        char* image;
    };

    // RTTI
    RTTI_DECLARATIONS(DrawableActor, Actor);

public:
    DrawableActor(const char* image = "");
    virtual ~DrawableActor();

    void Serialize(char* buffer);
    void Deserialize(const char* buffer);
    size_t SerializedSize() const;

    virtual void Draw() override;
    virtual void SetPosition(const Vector2& newPosition) override;

    // 충돌 확인 함수
    bool Intersect(const DrawableActor& other);

    int Width() const { return width; }

protected:

    // 화면에 그릴 문자 값
    char* image;

    // 너비(문자열 길이)
    int width = 0;

    // 색상 값
    Color color = Color::White;
};