#include "Projectile.h"

using namespace DirectX;

bool Projectile::Create(Model* projectileModel, Maze* maze)
{
    CHECK(projectileModel, false, "Cannot create a projectile with no model to render");
    CHECKSHOW(maze, "Creating a projectile without a maze");
    mProjectileModel = projectileModel;
    mMaze = maze;

    InstanceInfo info;
    info.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    info.WorldMatrix = XMMatrixIdentity();
    auto instanceResult = mProjectileModel->AddInstance(info);
    CHECK(instanceResult.Valid(), false, "Cannot add a projectile instance");
    mInstanceID = instanceResult.Get();

    return true;
}

void Projectile::Update(float dt)
{
    if (mActive)
    {
        mPosition = mPosition + mDirection * dt * speed;

        XMMATRIX worldMatrix = XMMatrixScaling(scale, scale, scale);
        worldMatrix *= XMMatrixTranslation(XMVectorGetX(mPosition), XMVectorGetY(mPosition), XMVectorGetZ(mPosition));

        mProjectileModel->GetInstanceInfo(mInstanceID).WorldMatrix = worldMatrix;

        mLifetime -= dt;
        if (mLifetime < 0.0f)
        {
            mActive = false;
        }

        if (mMaze)
        {
            BoundingBox currentBoundingBox;
            const auto& boundingBox = mProjectileModel->GetBoundingBox();
            boundingBox.Transform(currentBoundingBox, worldMatrix);
            if (mMaze->HandleCollisionBetweenBoundingBoxAndEnemies(currentBoundingBox))
            {
                SetActive(false);
            }
        }
    }
}

void Projectile::Render()
{
    if (mActive)
    {
        mProjectileModel->AddCurrentInstance(mInstanceID);
    }
}

void Projectile::SetActive(bool active, float lifetime)
{
    mActive = active;
    mLifetime = lifetime;
}

void __vectorcall Projectile::SetPosition(const XMVECTOR& position)
{
    mPosition = position;
}

void __vectorcall Projectile::SetDirection(const XMVECTOR& direction)
{
    mDirection = direction;
}

bool Projectile::IsActive() const
{
    return mActive;
}
