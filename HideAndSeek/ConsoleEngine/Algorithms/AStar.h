#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <iostream>

#include "Math/Vector2.h"

class Node
{
public:
	Vector2 position;

	Node() : position(0, 0), g(0.0f), h(0.0f), parent(nullptr) {}
	Node(Vector2& pos) : position(pos), g(0.0f), h(0.0f), parent(nullptr) {}

	float totalCost() const { return g + h; }

	Node* parent;
	float g; 
	float h; 
};

class AStar
{
public:
	static std::vector<Vector2> FindPath(Vector2 startPosition, Vector2 endPosition);

    static void SetMapData(std::vector<std::vector<char>>&& data);

private:
    static int Hash(Vector2 point) {
        return point.x * 73856093 ^ point.y * 19349663;
    }

    static float Heuristic(const Vector2& a, const Vector2& b)
    {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    struct Compare
    {
        bool operator() (const Node* a, const Node* b)
        {
            return a->totalCost() > b->totalCost();
        }
    };

private:
    static std::vector<std::vector<char>> mapData;
    static int mapWidth;
    static int mapHeight;
};