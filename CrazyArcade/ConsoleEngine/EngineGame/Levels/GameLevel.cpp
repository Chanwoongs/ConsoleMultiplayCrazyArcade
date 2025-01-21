#include "PrecompiledHeader.h"

#include "GameLevel.h"
#include "Math/Vector2.h"

#include "Engine/Engine.h"
#include "Engine/Timer.h"

#include "EngineGame/Actors/Ground.h"
#include "EngineGame/Actors/Wall.h"
#include "EngineGame/Actors/Player.h"

GameLevel::GameLevel()
{
    //system("cls");
    // 커서 감추기

    Engine::Get().SetCursorType(CursorType::NoCursor);

    map = new DrawableActor();
}

GameLevel::~GameLevel()
{
    for (auto& pos : wallPositions)
    {
        delete pos;
    }
}

void GameLevel::Update(float deltaTime)
{
    Super::Update(deltaTime);

    // 게임이 클리어됐으면, 게임 종료 처리
    if (isGameClear)
    {
        // 타이머
        static Timer timer(0.1f);
        timer.Update(deltaTime);
        if (!timer.IsTimeOut())
        {
            return;
        }

        // 커서 이동
        Engine::Get().SetCursorPosition(0, Engine::Get().ScreenSize().y);

        // 메세지 출력
        Log("Game Clear!");

        // 쓰레드 정지
        Sleep(2000);

        // 게임 종료 처리
        Engine::Get().QuitGame();
    }
}

void GameLevel::Draw()
{
    // 맵 그리기    
    if (map)
    {
        map->Draw();
    }
    
    // 플레이어 그리기
    for (auto& player : players)
    {
        player->Draw();
    }
}

bool GameLevel::CanPlayerMove(const Vector2& position)
{
    // 게임이 클리어된 경우 종료
    if (isGameClear)
    {
        return false;
    }

    // 이동하려는 위치에 벽이 있는 지 확인
    DrawableActor* searchedActor = nullptr;

    // 먼저 이동하려는 위치의 액터 찾기
    for (auto& pos : wallPositions)
    {
        if (*pos == position)
        {
            break;
        }
    }

    return false;
}

bool GameLevel::CheckGameClear()
{
    return false;
}

void GameLevel::LoadMap()
{
    // 맵 파일 불러와 업로드
    // 파일 읽기
    FILE* file = nullptr;
    fopen_s(&file, "../Assets/Maps/Map.txt", "rb");

    // 파일 처리
    if (file == nullptr)
    {
        std::cout << "맵 파일 열기 실패\n";
        __debugbreak();
        return;
    }

    // 파일 읽기 (이 방법은 스테이지가 작을 때 쓸 수 있는 방법, 스테이지가 크다면 짤라서 읽는 게 좋다)
    // 끝 위치로 이동
    fseek(file, 0, SEEK_END);

    // 이동한 위치의 FP 가져오기
    size_t readSize = ftell(file);

    // FP 원위치
    //fseek(file, 0, SEEK_SET); // set, cur, end
    rewind(file);

    // 파일 읽어서 버퍼에 담기
    char* buffer = new char[readSize + 1]; // 마지막 널 문자
    size_t bytesRead = fread(buffer, 1, readSize, file);

    if (readSize != bytesRead)
    {
        std::cout << "읽어온 크기가 다름\n";
        __debugbreak();
        return;
    }

    buffer[readSize] = '\0';
    map = new DrawableActor(buffer);

    // 파일 읽을 때 사용할 인덱스
    int index = 0;

    // 좌표 계산을 위한 변수 선언
    int xPosition = 0;
    int yPosition = 0;

    // 해석 (파싱-Parcing)
    while (index < (int)bytesRead)
    {
        char mapChar = buffer[index];
        index++;

        if (mapChar == '\n')
        {
            ++yPosition;
            xPosition = 0;
            continue;
        }

        if (mapChar == '1')
        {
            wallPositions.push_back(new Vector2(yPosition, xPosition));
        }
        ++xPosition;
    }

    delete[] buffer;

    fclose(file);
}

void GameLevel::SerializeGameState(char* buffer, size_t bufferSize, size_t& outSize)
{
    size_t offset = 0;

    size_t mapSize = 0;
    map->Serialize(buffer + offset, mapSize);
    offset += mapSize;

    outSize = offset;
}

void GameLevel::DeserializeGameState(const char* buffer)
{
    size_t offset = 0;

    size_t mapSize = 0;
    map->Deserialize(buffer + offset, mapSize);
    offset += mapSize;
}