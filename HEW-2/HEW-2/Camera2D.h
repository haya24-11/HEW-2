#pragma once
#include <SimpleMath.h>

class Camera2D
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector2* t) { target = t; }
    void SetPosition(const DirectX::SimpleMath::Vector2& p) { pos = p; }
    const DirectX::SimpleMath::Vector2& GetPosition() const { return pos; }

    // ✅ カメラの表示範囲（幅・高さ）を設定
    //  値が小さいほど「狭い範囲」(＝近くが見える) / 大きいほど広い範囲
    void SetViewSize(float w, float h)
    {
        viewW = (w < 1.0f) ? 1.0f : w;
        viewH = (h < 1.0f) ? 1.0f : h;
    }
    float GetViewW() const { return viewW; }
    float GetViewH() const { return viewH; }

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

    float viewW = 1280.0f; // ✅ 表示幅（初期値は適当にSCREEN_WIDTHに合わせてOK）
    float viewH = 720.0f;  // ✅ 表示高さ（初期値は適当にSCREEN_HEIGHTに合わせてOK）
};
