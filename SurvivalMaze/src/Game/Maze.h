#pragma once

#include "Oblivion.h"
#include "Model.h"
#include "Enemy.h"

class Maze {
public:
    struct MazeInitializationInfo {
        unsigned int rows = 10;
        unsigned int cols = 10;

        float tileWidthDepth = 10.f;

        Model* cubeModel;
        Model* enemyModel;
    };

public:
    Maze() = default;

public:
    Result<DirectX::XMFLOAT3> Create(const MazeInitializationInfo& info);
    void Update(float dt);
    void Render();
    void RenderDebug(BatchRenderer& batchRenderer);

    DirectX::XMFLOAT3 GetPositionFromCoordinates(const DirectX::XMINT2& coordinates) const;

    bool BoundingBoxCollidesWithWalls(const DirectX::BoundingBox& boundingBox) const;
    bool BoundingBoxCollidesWithEnemy(const DirectX::BoundingBox& boundingBox) const;

    bool HandleCollisionBetweenBoundingBoxAndEnemies(const DirectX::BoundingBox& boundingBox);

private:
    Result<DirectX::XMINT2> Lee();

    void AddModelInstances(uint32_t tileWidth, uint32_t tileDepth, Model* enemyModel);

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

    float mTileWidth, mTileDepth;
    

    std::vector<uint32_t> mTileInstances;
    std::vector<uint32_t> mWallInstances;

    std::vector<Enemy> mEnemies;
    std::vector<std::vector<TileType>> mTiles;
};
