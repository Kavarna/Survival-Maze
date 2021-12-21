#include "Maze.h"

Result<DirectX::XMINT2> Maze::Create(const MazeInitializationInfo& info)
{
    CHECK(info.rows >= 3 && info.cols >= 3, std::nullopt,
        "Can't create a maze with {} rows and {} cols. There should be at least 3 rows and at least 3 columns", info.rows, info.cols);
    CHECK(info.cubeModel != nullptr, std::nullopt, "A valid cube model is expected");
    mTiles.resize(info.rows);
    for (auto& row : mTiles) {
        row.resize(info.cols, TileType::Wall);
    }

    auto result = Lee();

#if DEBUG || _DEBUG
    PrintMazeToLogger();
#endif

    mCubeModel = info.cubeModel;
    AddModelInstances(info.tileWidth, info.tileDepth);

    return result;
}

Result<DirectX::XMINT2> Maze::Lee()
{
    DirectX::XMINT2 startPosition = {
        (int32_t)mTiles.size() / 2,
        (int32_t)mTiles[0].size() / 2
    };
    std::vector<decltype(startPosition)> st;
    st.push_back(startPosition);

    std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash> visitedTiles;
    visitedTiles.insert({ startPosition.y, startPosition.x });
    bool foundExit = false;

    while (!st.empty()) {
        auto currentPositionIndex = Random::get((size_t)0, st.size() - 1);
        auto currentPosition = st[currentPositionIndex];
        st.erase(st.begin() + currentPositionIndex);

        if (currentPosition.x == 0 || currentPosition.y == 0 || currentPosition.x == (mTiles[0].size() - 1) || currentPosition.y == (mTiles.size() - 1)) {
            mTiles[currentPosition.y][currentPosition.x] = TileType::Free;
            foundExit = true;
            break;
        }

        auto neighbours = GetNeighbours(currentPosition, visitedTiles);
        unsigned int freeTiles = 0;
        std::vector<DirectX::XMINT2> availableNeighbours;
        for (const auto& neighbour : neighbours) {
            const auto tile = mTiles[neighbour.y][neighbour.x];
            if (tile == TileType::Free) {
                freeTiles++;
            } else if (tile == TileType::Wall) {
                availableNeighbours.push_back(neighbour);
            }
        }
        if (freeTiles > 2) {
            continue;
        }

        if (Random::get(0.0f, 1.0f) <= 0.1f) {
            mTiles[currentPosition.y][currentPosition.x] = TileType::Enemy;
        } else {
            mTiles[currentPosition.y][currentPosition.x] = TileType::Free;
        }

        if (availableNeighbours.size() > 1) {
            st.push_back(currentPosition);
        }
        for (const auto nextNeighbour : availableNeighbours) {
            st.push_back(nextNeighbour);
            visitedTiles.insert({ nextNeighbour.y, nextNeighbour.x });
        }
    }

    SHOWINFO("Done generating maze");

    return startPosition;
}

void Maze::AddModelInstances(uint32_t tileWidth, uint32_t tileDepth)
{
    for (std::size_t i = 0; i < mTiles.size(); ++i) {
        for (std::size_t j = 0; j < mTiles[0].size(); ++j) {
            DirectX::XMFLOAT3 position, scale = { 2.0f, 2.0f, 2.0f };
            DirectX::XMFLOAT4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
            position.x = (j - mTiles[0].size()) * tileWidth;
            position.z = (i - mTiles.size()) * tileDepth;
            
            switch (mTiles[i][j]) {
            case TileType::Free:
            case TileType::Enemy:
                position.y = -1.0f;
                break;
            case TileType::Wall:
                position.y = 5.0f;
                scale.y = 20.f;
                color.x = 1.0f;
                color.y = 0.0f;
                color.z = 0.0f;
                break;
            default:
                break;
            }

            InstanceInfo instanceInfo;
            instanceInfo.WorldMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z) * DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
            instanceInfo.Color = color;
            mCubeModel->AddInstance(instanceInfo);
        }
    }
}

void Maze::PrintMazeToLogger()
{
    std::ostringstream mazeString;
    mazeString << "\n";
    for (const auto& row : mTiles) {
        for (const auto& col : row) {
            switch (col) {
            case TileType::Free:
                mazeString << ". ";
                break;
            case TileType::Wall:
                mazeString << "# ";
                break;
            case TileType::Enemy:
                mazeString << "? ";
                break;
            default:
                break;
            }
        }
        mazeString << "\n";
    }
    SHOWINFO("Generated maze is {}", mazeString.str());
}

std::vector<DirectX::XMINT2> Maze::GetNeighbours(const DirectX::XMINT2& tile, std::unordered_set<std::pair<uint32_t, uint32_t>, PairHash>& visitedTiles)
{
    constexpr int dirY[] = { 1, -1, 0, 0 };
    constexpr int dirX[] = { 0, 0, 1, -1 };
    static_assert(ARRAYSIZE(dirY) == ARRAYSIZE(dirX), "Direction arrays must have the same size");

    std::vector<DirectX::XMINT2> neighbours;
    for (uint32_t i = 0; i < ARRAYSIZE(dirY); ++i) {
        DirectX::XMINT2 newTile = tile;
        newTile.x += dirX[i];
        newTile.y += dirY[i];

        if (newTile.x < 0 || newTile.y < 0 || newTile.x > (mTiles[0].size() - 1) || newTile.y > (mTiles.size() - 1)) {
            continue;
        }
        if (visitedTiles.find({ newTile.y, newTile.x }) != visitedTiles.end()) {
            continue;
        }

        neighbours.push_back(newTile);
    }

    return neighbours;
}
