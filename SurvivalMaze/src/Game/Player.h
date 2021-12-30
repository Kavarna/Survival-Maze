#pragma once


#include "Oblivion.h"
#include "CompositeModel.h"
#include "Camera.h"
#include "Maze.h"
#include "ThirdPersonCamera.h"


OBLIVION_ALIGN(16)
class Player
{
    static constexpr const float distanceToWallInFrames = 2;
public:
    Player() = default;
    ~Player() = default;

public:
    bool Create(Model* usedModel, Maze* maze);
    void Render();
    void RenderDebug(BatchRenderer& renderer);

    bool Walk(float dt);
    bool Strafe(float dt);
    void HandleAnimation(float dt);
    void ResetAnimation();

    void SetCamera(ICamera* camera);

private:
    void ResetTransform();
    bool __vectorcall MoveDirection(float dt, DirectX::XMVECTOR actualDirection);
    float GetYAngle(const DirectX::XMVECTOR& actualDirection);
    bool __vectorcall PositionCollidesWithMaze(const DirectX::XMVECTOR& position, float angle);

public:
    ICamera* mCamera;
    Maze* mMaze;

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
