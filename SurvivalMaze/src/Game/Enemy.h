#pragma once



#include "Model.h"


class Enemy
{
    static const DirectX::XMVECTOR directions[];
public:
    Enemy() = default;

    bool Create(Model* enemyModel, DirectX::XMFLOAT3 position, float range);

    void Update(float dt);
    void Render();

    bool CollisionWithBoundingBox(const DirectX::BoundingBox& bb) const;

private:
    Model* mModel;
    float mRange;

    float mAnimationTime;

    DirectX::XMVECTOR mInitialPosition;
    DirectX::XMVECTOR mPosition;
    DirectX::XMVECTOR mDirection;

    uint32_t mInstanceID;
};

