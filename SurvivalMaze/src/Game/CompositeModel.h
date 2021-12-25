#pragma once


#include <Oblivion.h>
#include <Model.h>


class CompositeModel
{
public:
    CompositeModel() = default;

public:
    bool __vectorcall Create(Model* usedModel, const DirectX::XMFLOAT4& color,
        const DirectX::XMMATRIX& fromParent = DirectX::XMMatrixIdentity(),
        const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity());

    CompositeModel* __vectorcall AddChild(const DirectX::XMFLOAT4& color, const DirectX::XMMATRIX& fromParent = DirectX::XMMatrixIdentity(),
        const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity());

    void Render(const DirectX::XMMATRIX& compositeTransform = DirectX::XMMatrixIdentity());
    void RenderDebug(BatchRenderer& renderer);

public:
    void UpdateBoundingBox(const DirectX::XMMATRIX& compositeTransform = DirectX::XMMatrixIdentity());
    const DirectX::BoundingBox& GetBoundingBox() const;

public:
    void Identity();
    void Translate(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    void RotateX(float theta);
    void RotateY(float theta);
    void RotateZ(float theta);
    void Scale(float scaleFactor);
    void Scale(float scaleFactorX, float scaleFactorY, float scaleFactorZ);

public:
    void IdentityFromParent();
    void TranslateFromParent(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    void RotateXFromParent(float theta);
    void RotateYFromParent(float theta);
    void RotateZFromParent(float theta);
    void ScaleFromParent(float scaleFactor);
    void ScaleFromParent(float scaleFactorX, float scaleFactorY, float scaleFactorZ);

private:
    std::vector<std::unique_ptr<CompositeModel>> mChildren;
    Model* mUsedModel;
    uint32_t mInstanceID;

    DirectX::XMMATRIX mFromParentTransformation = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mTransform = DirectX::XMMatrixIdentity();

    DirectX::BoundingBox mBoundingBox;
};

