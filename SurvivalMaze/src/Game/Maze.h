#pragma once

#include "Oblivion.h"
#include "Model.h"

class Maze {
public:
    struct MazeInitializationInfo {
        unsigned int rows = 10;
        unsigned int cols = 10;

        float tileWidth = 10.f;
        float tileDepth = 10.f;

        Model* cubeModel;
    };

public:
    Maze() = default;

public:
    Result<DirectX::XMINT2> Create(const MazeInitializationInfo& info);

private:
    Result<DirectX::XMINT2> Lee();

    void AddModelInstances(uint32_t tileWidth, uint32_t tileDepth);

    void PrintMazeToLogger();

private:
    struct PairHash {
        inline std::size_t operator()(const std::pair<int, int>& v) const
        {
            return v.first * 31 + v.second;
        }
    };

    std::vector<DirectX::XMINT2> GetNeighbours(const DirectX::XMINT2& tile, std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& visitedTiles);

private:
    enum class TileType : unsigned char {
        Free = 0,
        Wall,
        Enemy,
    };
    DirectX::XMFLOAT2 mStartPosition;

    Model* mCubeModel = nullptr;
    

    std::vector<std::vector<TileType>> mTiles;
};
