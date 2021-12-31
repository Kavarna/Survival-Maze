#include "ProjectileManager.h"

bool ProjectileManager::Create(Model* projectileModel, Maze* maze, uint32_t maxNumProjectiles)
{
    mProjectiles.resize((size_t)maxNumProjectiles);
    for (auto& projectile : mProjectiles)
    {
        projectile.Create(projectileModel, maze);
    };
    return true;
}

void ProjectileManager::Update(float dt)
{
    for (auto& projectile : mProjectiles)
    {
        projectile.Update(dt);
    }
}

void ProjectileManager::Render()
{
    for (auto& projectile : mProjectiles)
    {
        projectile.Render();
    }
}

bool __vectorcall ProjectileManager::SpawnProjectile(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction)
{
    for (auto& projectile : mProjectiles)
    {
        if (!projectile.IsActive())
        {
            projectile.SetPosition(position);
            projectile.SetDirection(direction);
            projectile.SetActive(true);
            return true;
        }
    }
    return false;
}
