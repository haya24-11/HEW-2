// Camera2D.h
#pragma once
#include <SimpleMath.h>

class Camera2D
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector2* t) { target = t; }
    void SetPosition(const DirectX::SimpleMath::Vector2& p) { pos = p; }
    const DirectX::SimpleMath::Vector2& GetPosition() const { return pos; }

    // targetがあればtargetに移動
    void Update(float dt)
    {
        if (!target) return;
        // すぐにtargetに移動
        pos = *target;

    // スムーズにフォローしたい場合(オプション):
    // float k = 10.0f; // 付いて行く速度
    // pos += (*target - pos) * (1.0f - std::exp(-k * dt));
    }
    DirectX::SimpleMath::Vector2 GetOffset() const
    {
        return -pos;
    }

private:
    DirectX::SimpleMath::Vector2 pos{ 0,0 };
    const DirectX::SimpleMath::Vector2* target = nullptr;
};
