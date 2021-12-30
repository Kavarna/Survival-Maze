#pragma once



#include "Projectile.h"



class ProjectileManager
{
public:
    bool Create(Model* projectileModel, uint32_t maxNumProjectiles);
    void Update(float dt);
    void Render();

    bool __vectorcall SpawnProjectile(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& direction);

private:
    std::vector<Projectile> mProjectiles;

};

