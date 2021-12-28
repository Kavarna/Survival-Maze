#include "CompositeModel.h"


bool CompositeModel::Create(Model* usedModel, const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& fromParent, const DirectX::XMMATRIX& transform)
{
    mUsedModel = usedModel;
    mFromParentTransformation = fromParent;
    mTransform = transform;
    InstanceInfo instanceInfo = {};
    instanceInfo.Color = color;
    instanceInfo.WorldMatrix = fromParent * transform;
    
    auto idResult = mUsedModel->AddInstance(instanceInfo);
    CHECK(idResult.Valid(), false, "Cannot add new instance to composite model");
    mInstanceID = idResult.Get();

    return true;
}

CompositeModel* __vectorcall CompositeModel::AddChild(const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& fromParent, const DirectX::XMMATRIX& transform)
{
    mChildren.emplace_back(std::make_unique<CompositeModel>());
    auto& child = mChildren.back();
    if (!child->Create(mUsedModel, color, fromParent, transform))
    {
        SHOWFATAL("Unable to add child to composite model");
        mChildren.pop_back();
    }

    return child.get();
}

void CompositeModel::Render(const DirectX::XMMATRIX& compositeTransform)
{
    auto& worldMatrix = mUsedModel->GetInstanceInfo(mInstanceID).WorldMatrix;
    worldMatrix = mFromParentTransformation * mTransform * compositeTransform;

    mUsedModel->AddCurrentInstance(mInstanceID);

    for (auto& child : mChildren)
    {
        child->Render(worldMatrix);
    }
}

void CompositeModel::RenderDebug(BatchRenderer& renderer)
{
    DirectX::BoundingBox worldBoundingBox;
    mBoundingBox.Transform(worldBoundingBox, mTransform);
    renderer.BoundingBox(worldBoundingBox, DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
}

void CompositeModel::UpdateBoundingBox(const DirectX::XMMATRIX& compositeTransform)
{
    const auto& boundingBox = mUsedModel->GetBoundingBox();
    auto currentTransform = mFromParentTransformation * compositeTransform;
    boundingBox.Transform(mBoundingBox, currentTransform);
    for (auto& child : mChildren)
    {
        child->UpdateBoundingBox(currentTransform);
        
        DirectX::BoundingBox::CreateMerged(mBoundingBox, mBoundingBox, child->mBoundingBox);
    }
}

const DirectX::BoundingBox& CompositeModel::GetBoundingBox() const
{
    return mBoundingBox;
}

void CompositeModel::Identity()
{
    mTransform = DirectX::XMMatrixIdentity();
}

void CompositeModel::Translate(float x, float y, float z)
{
    mTransform *= DirectX::XMMatrixTranslation(x, y, z);
}

void CompositeModel::RotateX(float theta)
{
    mTransform *= DirectX::XMMatrixRotationX(theta);
}

void CompositeModel::RotateY(float theta)
{
    mTransform *= DirectX::XMMatrixRotationY(theta);
}

void CompositeModel::RotateZ(float theta)
{
    mTransform *= DirectX::XMMatrixRotationZ(theta);
}

void CompositeModel::Scale(float scaleFactor)
{
    Scale(scaleFactor, scaleFactor, scaleFactor);
}

void CompositeModel::Scale(float x, float y, float z)
{
    mTransform *= DirectX::XMMatrixScaling(x, y, z);
}

void CompositeModel::IdentityFromParent()
{
    mFromParentTransformation = DirectX::XMMatrixIdentity();
}

void CompositeModel::TranslateFromParent(float x, float y, float z)
{
    mFromParentTransformation *= DirectX::XMMatrixTranslation(x, y, z);
}

void CompositeModel::RotateXFromParent(float theta)
{
    mFromParentTransformation *= DirectX::XMMatrixRotationX(theta);
}

void CompositeModel::RotateYFromParent(float theta)
{
    mFromParentTransformation *= DirectX::XMMatrixRotationY(theta);
}

void CompositeModel::RotateZFromParent(float theta)
{
    mFromParentTransformation *= DirectX::XMMatrixRotationZ(theta);
}

void CompositeModel::ScaleFromParent(float scaleFactor)
{
    ScaleFromParent(scaleFactor, scaleFactor, scaleFactor);
}

void CompositeModel::ScaleFromParent(float scaleFactorX, float scaleFactorY, float scaleFactorZ)
{
    mFromParentTransformation *= DirectX::XMMatrixScaling(scaleFactorX, scaleFactorY, scaleFactorZ);
}
