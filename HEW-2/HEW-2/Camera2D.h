#pragma once
#include <SimpleMath.h>

class Camera2D
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector2* t) { target = t; }
    void SetPosition(const DirectX::SimpleMath::Vector2& p) { pos = p; }
    const DirectX::SimpleMath::Vector2& GetPosition() const { return pos; }

    // ✅ View Size / Zoom
    void SetViewSize(float w, float h)
    {
        if (w < 1.0f) w = 1.0f;
        if (h < 1.0f) h = 1.0f;
        baseViewW = w;
        baseViewH = h;
    }

    void SetZoom(float z)
    {
        if (z < 0.05f) z = 0.05f;
        zoom = z;
    }
    float GetZoom() const { return zoom; }

    float GetViewW() const { return baseViewW / zoom; }
    float GetViewH() const { return baseViewH / zoom; }

    void Update(float dt)
    {
        if (!target) return;
        pos = *target;
    }

    DirectX::SimpleMath::Vector2 GetOffset() const
    {
        return -pos;
    }

private:
    DirectX::SimpleMath::Vector2 pos{ 0,0 };
    const DirectX::SimpleMath::Vector2* target = nullptr;

    float baseViewW = 640.0f;
    float baseViewH = 320.0f;
    float zoom = 1.0f;
};
