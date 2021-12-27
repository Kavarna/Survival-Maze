#pragma once


#include "Oblivion.h"
#include "CompositeModel.h"


class Player
{
public:
    Player() = default;
    ~Player() = default;

public:
    bool Create(Model* usedModel);
    void Render();

    void Walk(float dt);
    void ResetTransform();

private:
    CompositeModel mModel;

    CompositeModel* mHead;
    CompositeModel* mRightShoulder;
    CompositeModel* mLeftShoulder;
    CompositeModel* mRightLeg;
    CompositeModel* mLeftLeg;

    float mAnimationSpeed = 3.0f;
    float mAnimationTime = 0.0f;
    int mAnimationDelta = 1;

    DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();
};
