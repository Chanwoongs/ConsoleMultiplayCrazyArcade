#include "PrecompiledHeader.h"
#include "DrawableActor.h"
#include "Engine/Engine.h"

DrawableActor::DrawableActor(Vector2 position, const char* image)
    : Actor()
{
    this->position = position;

    auto length = strlen(image) + 1;
    this->image = new char[length];
    strcpy_s(this->image, length, image);

    // 너비 설정
    width = (int)strlen(image);
}

DrawableActor::~DrawableActor()
{
    delete[] image;
}

void DrawableActor::Serialize(char* buffer, size_t& size)
{
    Super::Serialize(buffer, size);

    size_t offset = 0;

    memcpy(buffer + offset, &width, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer + offset, &color, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint32_t imageLength = (uint32_t)strlen(image) + 1;
    memcpy(buffer + offset, &imageLength, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer + offset, image, imageLength);
    offset += imageLength;

    size = offset;
}

void DrawableActor::Deserialize(const char* buffer, size_t& size)
{
    Super::Deserialize(buffer, size);

    size_t offset = 0;

    memcpy(&width, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

     memcpy(&color, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    uint32_t imageLength = 0;
    memcpy(&imageLength, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    delete[] image; 
    image = new char[imageLength];
    memset(image, 0, imageLength);
    memcpy(image, buffer + offset, imageLength);
    offset += imageLength;

    size = offset;
 }

void DrawableActor::Draw()
{
    Super::Draw();

    // 색상 설정
    SetColor(color);

    // 그리기
    // 1단계: 콘솔 좌표 옮기기
    Engine::Get().SetCursorPosition(position);

    // 2단계: 그리기 (콘솔 출력)
    Log(image);

    // 색상 설정
    SetColor(Color::White);

    Engine::Get().SetCursorPosition(Vector2(0, 0));
}

void DrawableActor::SetPosition(const Vector2& newPosition)
{
    //// 이전의 위치를 먼저 지우기
    //Engine::Get().SetCursorPosition(position);
    //Log(" ");

    // 위치를 새로 옮기기
    Super::SetPosition(newPosition);
}

void DrawableActor::SetImage(const char* newImage)
{
    if (newImage == nullptr)
        return;

    delete[] image;

    size_t imageLength = strlen(newImage);

    image = new char[imageLength + 1]; 

    strcpy_s(image, imageLength + 1,newImage);

    width = (int)imageLength;
}

bool DrawableActor::Intersect(const DrawableActor& other)
{
    // AABB (Axis Aligned Bounding Box)

    // 내 x좌표 최소 / 최대
    int min = position.x;
    int max = position.x + width;

    // 다른 액터의 x좌표 최소 / 최대
    int otherMin = other.position.x;
    int otherMax = other.position.x + other.width;

    // 다른 액터의 왼쪽 끝 위치가 내 오른쪽 끝 위치를 벗어나면 충돌 안함
    if (otherMin > max)
    {
        return false;
    }
    // 다른 액터의 오른쪽 끝 위치가 내 왼쪽 끝 위치보다 작으면 충돌 안함
    if (otherMax < min)
    {
        return false;
    }
    // 위의 두 경우가 아니라면 (x좌표는 서로 겹침), y위치 비교
    return position.y == other.position.y;
}
