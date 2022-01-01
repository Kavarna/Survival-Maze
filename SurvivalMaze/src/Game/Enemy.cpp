#include "Enemy.h"

using namespace DirectX;

const XMVECTOR Enemy::directions[] = {
    XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f),
    XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
    XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f),
};

bool Enemy::Create(Model* enemyModel, XMFLOAT3 position, float range)
{
    CHECK(enemyModel, false, "Unable to create an enemy with an empty model");
    mModel = enemyModel;

    const auto& boundingBox = enemyModel->GetBoundingBox();

    mRange = range;
    float halfHeight = boundingBox.Extents.y;
    position.y += halfHeight;
    InstanceInfo instanceInfo;
    instanceInfo.Color = { 0.0f, 1.0f, 0.0f,1.0f };
    instanceInfo.WorldMatrix = XMMatrixTranslation(position.x, position.y, position.z);
    
    mInitialPosition = XMLoadFloat3(&position);
    
    mAnimationTime = 1.0f; // Count on Update to update everything

    auto instanceResult = enemyModel->AddInstance(instanceInfo);
    CHECK(instanceResult.Valid(), false, "Cannot add new instance to enemy model");

    mInstanceID = instanceResult.Get();

    return true;
}

void Enemy::Update(float dt)
{
    if (mDying)
    {
        mAnimationTime += dt * 2.0f;

        InstanceInfo& instanceInfo = mModel->GetInstanceInfo(mInstanceID);
        instanceInfo.AnimationTime = mAnimationTime;
    }
    else
    {
        if (mAnimationTime >= 1.0f)
        {
            uint32_t nextDirectionIndex = Random::get(0u, (uint32_t)ARRAYSIZE(directions) - 1);
            mDirection = directions[nextDirectionIndex];
            mAnimationTime = 0.0f;
        }
        mAnimationTime += dt * Random::get(0.0001f, 1.0f / 3.0f); // speed between 0.0001f and 1/3

        InstanceInfo& instanceInfo = mModel->GetInstanceInfo(mInstanceID);

        XMVECTOR position = mInitialPosition + mDirection * sin(mAnimationTime * XM_2PI);
        instanceInfo.WorldMatrix = XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
    }
}

void Enemy::Render()
{
    mModel->AddCurrentInstance(mInstanceID);
}

void Enemy::Die()
{
    mDying = true;
    mAnimationTime = 0.0f;
}

bool Enemy::CollisionWithBoundingBox(const DirectX::BoundingBox& bb) const
{
    if (mDying)
        return false;

    InstanceInfo& instanceInfo = mModel->GetInstanceInfo(mInstanceID);
    const auto& boundingBox = mModel->GetBoundingBox();
    BoundingBox currentBoundingBox;
    boundingBox.Transform(currentBoundingBox, instanceInfo.WorldMatrix);

    return currentBoundingBox.Intersects(bb);
}

bool Enemy::ShouldDie() const
{
    return mDying && mAnimationTime >= 1.0f;
}
