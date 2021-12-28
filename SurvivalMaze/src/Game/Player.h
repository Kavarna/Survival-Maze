#pragma once


#include "Oblivion.h"
#include "CompositeModel.h"
#include "Camera.h"
#include "ThirdPersonCamera.h"


OBLIVION_ALIGN(16)
class Player
{
public:
    Player() = default;
    ~Player() = default;

public:
    bool Create(Model* usedModel);
    void Render();
    void RenderDebug(BatchRenderer& renderer);

    bool Walk(float dt);
    bool Strafe(float dt);
    void HandleAnimation(float dt);
    void ResetAnimation();

    void SetCamera(ICamera* camera);

private:
    void ResetTransform();
    bool MoveForward(float dt);
    float GetYAngle(const DirectX::XMVECTOR& actualDirection);
    bool MoveRight(float dt);

public:
    ICamera* mCamera;

    CompositeModel mModel;

    CompositeModel* mHead;
    CompositeModel* mRightShoulder;
    CompositeModel* mLeftShoulder;
    CompositeModel* mRightLeg;
    CompositeModel* mLeftLeg;

    DirectX::XMVECTOR mPosition;
    float mYAngle = 0.0f;

    float mMoveSpeed = 3.0f;
    float mAnimationSpeed = 3.0f;
    float mAnimationTime = 0.0f;
    int mAnimationDelta = 1;
};
