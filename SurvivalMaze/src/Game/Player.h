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

private:
    CompositeModel mModel;

    CompositeModel* mHead;
    CompositeModel* mRightShoulder;
    CompositeModel* mLeftShoulder;
    CompositeModel* mRightLeg;
    CompositeModel* mLeftLeg;

    DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();
};
