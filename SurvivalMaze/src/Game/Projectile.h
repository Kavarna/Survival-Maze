#pragma once


#include <Model.h>


class Projectile
{
public:
    bool Create(Model* projectileModel);
    void Update(float dt);
    void Render();

    void SetActive(bool active, float lifetime = 5.0f);

    void __vectorcall SetPosition(const DirectX::XMVECTOR& position);
    void __vectorcall SetDirection(const DirectX::XMVECTOR& direction);

    bool IsActive() const;

private:
    static constexpr const float scale = 0.5f;
    static constexpr const float speed = 5.0f;

    Model* mProjectileModel;

    DirectX::XMVECTOR mPosition;
    DirectX::XMVECTOR mDirection;

    bool mActive = false;
    float mLifetime = 0.0f;

    uint32_t mInstanceID;
};


