#include "Player.h"

using namespace DirectX;

bool Player::Create(Model* usedModel)
{
    mPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    // Best way to skin a mesh :>
    CHECK(mModel.Create(usedModel, DirectX::XMFLOAT4(0.25f, 0.87f, 0.81f, 1.0f)), false, "Unable to create player composite");
    mModel.ScaleFromParent(1.0f, 1.0f, 0.5f);

    mHead = mModel.AddChild(DirectX::XMFLOAT4(1.0f, 0.80f, 0.70f, 1.0f));
    CHECK(mHead, false, "Unable to add head to player composite model");
    mHead->TranslateFromParent(0.0f, 1.6f, 0.0f);
    mHead->ScaleFromParent(0.5f);

    mRightShoulder = mModel.AddChild(DirectX::XMFLOAT4(0.25f, 0.87f, 0.81f, 1.0f));
    CHECK(mRightShoulder, false, "Unable to add right shoulder to player composite model");
    mRightShoulder->TranslateFromParent(1.6f, 0.5f, 0.0f);
    mRightShoulder->ScaleFromParent(0.5f);

    auto rightArm = mRightShoulder->AddChild(DirectX::XMFLOAT4(1.0f, 0.80f, 0.70f, 1.0f));
    CHECK(rightArm, false, "Unable to add right arm to player composite model");
    rightArm->TranslateFromParent(0.0f, -1.1f, 0.0f);

    mRightLeg = mModel.AddChild(DirectX::XMFLOAT4(0.25, 0.25f, 1.0f, 1.0f));
    CHECK(mRightLeg, false, "Unable to add right leg to player composite model");
    mRightLeg->ScaleFromParent(0.4f, 1.0f, 0.75f);
    mRightLeg->TranslateFromParent(0.3f, -1.1f, 0.0f);

    mLeftShoulder = mModel.AddChild(DirectX::XMFLOAT4(0.25f, 0.87f, 0.81f, 1.0f));
    CHECK(mLeftShoulder, false, "Unable to add left shoulder to player composite model");
    mLeftShoulder->TranslateFromParent(-1.6f, 0.5f, 0.0f);
    mLeftShoulder->ScaleFromParent(0.5f);

    auto leftArm = mLeftShoulder->AddChild(DirectX::XMFLOAT4(1.0f, 0.80f, 0.70f, 1.0f));
    CHECK(leftArm, false, "Unable to add left arm to player composite model");
    leftArm->TranslateFromParent(0.0f, -1.1f, 0.0f);

    mLeftLeg = mModel.AddChild(DirectX::XMFLOAT4(0.25, 0.25f, 1.0f, 1.0f));
    CHECK(mLeftLeg, false, "Unable to add right leg to player composite model");
    mLeftLeg->ScaleFromParent(0.4f, 1.0f, 0.75f);
    mLeftLeg->TranslateFromParent(-0.3f, -1.1f, 0.0f);

    mModel.UpdateBoundingBox();

    return true;
}

void Player::Render()
{
    mModel.Identity();
    mModel.RotateY(mYAngle);
    mModel.Translate(XMVectorGetX(mPosition), XMVectorGetY(mPosition), XMVectorGetZ(mPosition));
    mModel.Render();
}

void Player::RenderDebug(BatchRenderer& renderer)
{
    mModel.RenderDebug(renderer);
}

bool Player::Walk(float dt)
{
    HandleAnimation(dt);
    return MoveForward(dt);
}

bool Player::Strafe(float dt)
{
    HandleAnimation(dt);
    return MoveRight(dt);
}

void Player::HandleAnimation(float dt)
{
    ResetTransform();
    dt = fabs(dt); // use the same animation for forward & backward walking since the model is symmetrical
    mAnimationTime += dt * mAnimationDelta * mAnimationSpeed;

    if (mAnimationTime >= DirectX::XM_PIDIV4)
    {
        mAnimationDelta = -1;
    }
    else if (mAnimationTime <= -DirectX::XM_PIDIV4)
    {
        mAnimationDelta = 1;
    }

    mHead->RotateY(mAnimationTime * 0.5f);

    mRightShoulder->RotateX(mAnimationTime);
    mLeftShoulder->RotateX(-mAnimationTime);

    mRightLeg->Translate(0.0f, -1.0f, 0.0f);
    mRightLeg->RotateX(-mAnimationTime * 0.40f);
    mRightLeg->Translate(0.0f, 1.0f, 0.0f);

    mLeftLeg->Translate(0.0f, -1.0f, 0.0f);
    mLeftLeg->RotateX(mAnimationTime * 0.40f);
    mLeftLeg->Translate(0.0f, 1.0f, 0.0f);
}

void Player::ResetAnimation()
{
    ResetTransform();
    mAnimationTime = 0.0f;
}

void Player::SetCamera(ICamera* camera)
{
    mCamera = camera;
}

void Player::ResetTransform()
{
    mHead->Identity();

    mRightShoulder->Identity();
    mLeftShoulder->Identity();
    
    mRightLeg->Identity();
    mLeftLeg->Identity();
}

bool Player::MoveForward(float dt)
{
    auto actualDirection = mCamera->GetDirection();
    actualDirection = XMVectorSetY(actualDirection, 0.0f);
    actualDirection = XMVector3Normalize(actualDirection);
    
    mPosition = mPosition + actualDirection * dt * mMoveSpeed;
    
    mYAngle = GetYAngle(actualDirection);

    if (auto thirdPersonCamera = dynamic_cast<ThirdPersonCamera*>(mCamera); thirdPersonCamera)
    {
        thirdPersonCamera->SetTarget(mPosition);
    }

    return true;
}

float Player::GetYAngle(const DirectX::XMVECTOR& actualDirection)
{
    float zDirection = XMVectorGetZ(actualDirection);
    XMVECTOR v1, v2;
    v1 = XMVectorSet(XMVectorGetX(actualDirection), zDirection, 0.0f, 1.0f);
    v2 = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f); // Forward direction projected in XZ coordinates
    float rawAngle;
    rawAngle = XMVectorGetX(XMVector2AngleBetweenNormals(v1, v2));

    float angleToRight;
    v2 = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f); // Right direction projected in XZ coordinates
    angleToRight = XMVectorGetX(XMVector2AngleBetweenNormals(v1, v2));

    float angleToLeft;
    v2 = XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f); // Left direction projected in XZ coordinates
    angleToLeft = XMVectorGetX(XMVector2AngleBetweenNormals(v1, v2));

    float finalAngle;
    if (angleToRight >= angleToLeft)
    {
        finalAngle = -rawAngle;
    }
    else
    {
        finalAngle = rawAngle;
    }

    return finalAngle;
}

bool Player::MoveRight(float dt)
{
    auto actualDirection = mCamera->GetRightDirection();
    actualDirection = XMVectorSetY(actualDirection, 0.0f);
    actualDirection = XMVector3Normalize(actualDirection);

    mPosition = mPosition + actualDirection * dt * mMoveSpeed;

    mYAngle = GetYAngle(actualDirection);

    if (auto thirdPersonCamera = dynamic_cast<ThirdPersonCamera*>(mCamera); thirdPersonCamera)
    {
        thirdPersonCamera->SetTarget(mPosition);
    }

    return true;
}
