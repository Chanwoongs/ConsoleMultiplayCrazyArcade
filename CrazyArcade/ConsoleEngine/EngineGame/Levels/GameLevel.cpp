#include "PrecompiledHeader.h"

#include "GameLevel.h"

#include "Engine/Engine.h"
#include "Engine/Timer.h"

GameLevel::GameLevel()
{
    system("cls");
    // 커서 감추기
    Engine::Get().SetCursorType(CursorType::NoCursor);

    LoadMap();
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
    for (auto& actor : map)
    {
        actor->Draw();  
    }
    
    // 플레이어 그리기
    //player->Draw();
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
    for (auto& actor : map)
    {
        if (actor->Position() == position)
        {
            searchedActor = actor;
            break;
        }
    }

    // 검색한 액터가 벽인지 확인
    if (searchedActor->As<Wall>())
    {
        return false;
    }

    // 검색한 액터가 이동 가능한 액터 (땅/타겟)인지 확인
    if (searchedActor->As<Ground>())
    {
        return true;
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

    // 파일 읽을 때 사용할 인덱스
    int index = 0;

    // 좌표 계산을 위한 변수 선언
    int xPosition = 0;
    int yPosition = 0;

    // 해석 (파싱-Parcing)
    while (index < (int)bytesRead)
    {
        char mapChar = buffer[index++];

        if (mapChar == '\n')
        {
            ++yPosition;
            xPosition = 0;
            continue;
        }
        if (mapChar == '1')
        {
            Wall* wall = new Wall(Vector2(xPosition, yPosition));
            actors.PushBack(wall);
            map.push_back(wall);
        }
        else if (mapChar == ' ')
        {
            Ground* ground = new Ground(Vector2(xPosition, yPosition));
            actors.PushBack(ground);
            map.push_back(ground);
        }
        ++xPosition;
    }

    delete[] buffer;

    fclose(file);
}