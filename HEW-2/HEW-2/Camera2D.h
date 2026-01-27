// Camera2D.h
#pragma once
#include <SimpleMath.h>

class Camera2D
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector2* t) { target = t; }
    void SetPosition(const DirectX::SimpleMath::Vector2& p) { pos = p; }
    const DirectX::SimpleMath::Vector2& GetPosition() const { return pos; }

    // target이 있으면 따라가기(스무딩 옵션 포함)
    void Update(float dt)
    {
        if (!target) return;

        // 즉시 따라가기:
        pos = *target;

        // 부드럽게 따라가기 하고 싶으면(옵션):
        // float k = 10.0f; // 따라가는 속도
        // pos += (*target - pos) * (1.0f - std::exp(-k * dt));
    }

    // 월드좌표 -> 화면좌표 변환용 오프셋
    DirectX::SimpleMath::Vector2 GetOffset() const
    {
        return -pos;
    }

private:
    DirectX::SimpleMath::Vector2 pos{ 0,0 };
    const DirectX::SimpleMath::Vector2* target = nullptr;
};
