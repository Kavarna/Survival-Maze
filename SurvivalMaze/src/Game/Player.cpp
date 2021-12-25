#include "Player.h"

bool Player::Create(Model* usedModel)
{
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
    mRightLeg->ScaleFromParent(0.4f, 1.0f, 1.0f);
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
    mLeftLeg->ScaleFromParent(0.4f, 1.0f, 1.0f);
    mLeftLeg->TranslateFromParent(-0.3f, -1.1f, 0.0f);

    mModel.UpdateBoundingBox();

    return true;
}

void Player::Render()
{
    mModel.Render(mWorld);
}

void Player::Walk(float dt)
{
    mHead->RotateY(dt);

    mRightShoulder->RotateX(dt);
    mLeftShoulder->RotateX(-dt);
}
