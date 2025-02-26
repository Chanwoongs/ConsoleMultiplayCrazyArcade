#include "PrecompiledHeader.h"

#include "AStar.h"

std::vector<std::vector<char>> AStar::mapData;
int AStar::mapWidth = 0;
int AStar::mapHeight = 0;

void AStar::SetMapData(std::vector<std::vector<char>>&& data)
{
    mapData = std::move(data);

    mapHeight = (int)mapData.size();
    mapWidth = (int)mapData[0].size();
}

std::vector<Vector2*> AStar::FindPath(Vector2 startPosition, Vector2 endPosition)
{
    if (endPosition.x > mapWidth || endPosition.y > mapHeight)
    {
        return std::vector<Vector2*>();
    }

	std::priority_queue<Node*, std::vector<Node*>, Compare> openList;
    std::unordered_map<int, Node*> allNodes;

	Node* startNode = new Node(startPosition);
	openList.push(startNode);
	allNodes[Hash(startPosition)] = startNode;

	while (!openList.empty()) 
    {
		// 비용이 가장 낮은 노드 선택
		Node* current = openList.top();
		openList.pop();

		// 경로를 찾았다
		if (current->position.x == endPosition.x && current->position.y == endPosition.y)
		{
            std::vector<Vector2*> path;

			// 경로를 벡터에 담고
			while (current->parent)
			{
				path.push_back(new Vector2(
					current->position.x,
					current->position.y));
				current = current->parent;
			}
			// 역순으로 
            std::reverse(path.begin(), path.end());

            // 모든 동적으로 할당된 노드 해제
            for (auto& pair : allNodes)
            {
                delete pair.second;
            }

			return path;
		}

		// 주변 노드 체크 (8방향)
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				Vector2 newPosition;
				newPosition.x = current->position.x + x;
				newPosition.y = current->position.y + y;

				// 재방문 방지
				if (newPosition.x == current->position.x &&
					newPosition.y == current->position.y) continue;
				// 범위 체크
				if (newPosition.x < 0 || newPosition.y < 0 ||
					newPosition.x >= mapWidth || newPosition.y >= mapHeight) continue;
				// 갈 수 없는 곳 체크
				if (mapData[newPosition.y][newPosition.x] == '1') continue;

				Node* newNode = new Node(newPosition);

				float newCost = current->g + Heuristic(current->position, newPosition);
				if (allNodes.find(Hash(newPosition)) == allNodes.end() ||
					newCost < allNodes[Hash(newPosition)]->g)
				{
					newNode->g = newCost;
					newNode->h = Heuristic(newPosition, endPosition);
					newNode->parent = allNodes[Hash(current->position)];

					allNodes[Hash(newPosition)] = newNode;
					openList.push(newNode);
				}
                else
                {
                    delete newNode; // 사용되지 않는 노드 해제
                }
			}
		}
	}

    // 모든 동적으로 할당된 노드 해제
    for (auto& pair : allNodes)
    {
        delete pair.second;
    }
	
	return std::vector<Vector2*>(); // 경로를 찾을 수 없음
}