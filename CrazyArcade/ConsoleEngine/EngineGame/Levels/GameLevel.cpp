#include "PrecompiledHeader.h"

#include "GameLevel.h"
#include "Math/Vector2.h"

#include "Engine/Engine.h"
#include "Engine/Timer.h"

#include "EngineGame/Actors/Ground.h"
#include "EngineGame/Actors/Wall.h"
#include "EngineGame/Actors/Player.h"

#include "Network/Packets.h"

GameLevel::GameLevel()
{
    system("cls");
    // 커서 감추기

    Engine::Get().SetCursorType(CursorType::NoCursor);

    map = new DrawableActor(Vector2(0, 0));
}

GameLevel::~GameLevel()
{
    delete map;

    for (auto& player : players)
    {
        delete player;
    }

    for (auto& pos : wallPositions)
    {
        delete pos;
    }
}

void GameLevel::AddActor(Actor* newActor)
{
    //Super::AddActor(newActor);

    if (newActor->As<Player>())
    {
        players.push_back(static_cast<Player*>(newActor));
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

    if (!isThreadWriting)
    {
        if (tempPlayers.size() > 0)
        {
            if (players.size() > 0)
            {
                for (Actor* actor : players)
                {
                    delete actor;
                }

                players.clear();
            }

            for (Player* player : tempPlayers)
            {
                players.push_back(player);
            }

            tempPlayers.clear();
        }
    }

    if (players.size() > 0)
    {
        for (auto& player : players)
        {
            player->Update(deltaTime);
        }
    }

    if (!isThreadWriting)
    {
        if (tempMap != nullptr)

        {
            delete map;
            map = tempMap;
            tempMap = nullptr;
        }
    }   
}

void GameLevel::Draw()
{
    // 맵 그리기    
    if (map != nullptr)
    {
        map->Draw();
    }
    
    // 플레이어 그리기
    if (players.size() > 0)
    {
        for (auto* player : players)
        {
            if (player != nullptr)
            {
                player->Draw();
            }
        }
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

void GameLevel::MovePlayer(int playerId, Direction direction)
{
    //moveQueue.push(std::make_pair(playerId, direction));

    //if (!moveQueue.empty())
    //{
    //    auto move = moveQueue.front();
    //    moveQueue.pop();

    //    Vector2 position = players[move.first]->Position();

    //    switch (move.second)
    //    {
    //    case Direction::UP:
    //        --position.y;
    //        break;
    //    case Direction::DOWN:
    //        ++position.y;
    //        break;
    //    case Direction::RIGHT:
    //        ++position.x;
    //        break;
    //    case Direction::LEFT:
    //        --position.x;
    //        break;
    //    }

    //    players[move.first]->SetPosition(position);
    //}

    if (playerId <= 0) return;

    WaitForSingleObject(mutex, INFINITE);

    if (playerId > players.size())
    {
        ReleaseMutex(mutex);
        return;
    }

    Vector2 position = players[playerId - 1]->Position();

    switch (direction)
    {
    case Direction::UP:
        --position.y;
        break;
    case Direction::DOWN:
        ++position.y;
        break;
    case Direction::RIGHT:
        ++position.x;
        break;
    case Direction::LEFT:
        --position.x;
        break;
    }

    players[playerId - 1]->SetPosition(position);

    ReleaseMutex(mutex);
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

    map->SetImage(buffer);

    // 파일 읽을 때 사용할 인덱 스
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

    //size_t size = 0;
    //map->Deserialize(buffer, size);

    uint32_t playerCount = static_cast<uint32_t>(players.size());
    memcpy(buffer + offset, &playerCount, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    for (auto* player : players)
    {
        size_t playerSize = 0;
        player->Serialize(buffer + offset, playerSize);
        offset += playerSize;
    }

    outSize = offset;
}

void GameLevel::DeserializeGameState(const char* buffer)
{
    if (tempPlayers.size() > 0 || tempMap != nullptr)
    {
        return;
    }

    isThreadWriting = true;

    size_t offset = 0;

    //size_t mapSize = 0;
    //map->Deserialize(buffer + offset, mapSize);
    //offset += mapSize;  

    size_t mapSize = 0;
    tempMap = new DrawableActor(Vector2(0, 0));
    tempMap->Deserialize(buffer + offset, mapSize);
    offset += mapSize;

    uint32_t playerCount = 0;
    memcpy(&playerCount, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    //for (auto* player : players)
    //{
    //    delete player;
    //}
    //players.clear();

    for (uint32_t i = 0; i < playerCount; ++i)
    {
        size_t playerSize = 0;
        auto* newPlayer = new Player(0, Vector2(0, 0), this);
        newPlayer->Deserialize(buffer + offset, playerSize);
        tempPlayers.push_back(newPlayer);
        offset += playerSize;

        //AddActor(newPlayer);
        //offset += playerSize;
    }

    isThreadWriting = false;
}