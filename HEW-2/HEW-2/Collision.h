#pragma once

#include <DirectXMath.h>

/*
    Collision
    =========
    ・2Dの「円」コライダ。
    ・Object の m_pos(x,y) と m_size(x,y) を同期して使う想定。

    使い方（例）
    ----------
    Collision a;
    a.SetPosition(0.0f, 0.0f);
    a.SetRadius(50.0f);

    Collision b;
    b.SetPosition(60.0f, 0.0f);
    b.SetRadius(50.0f);

    if (a.Intersects(b)) { ... }
*/

class Collision
{
public:
    Collision() = default;

    void SetPosition(float x, float y);
    void SetPosition(const DirectX::XMFLOAT2& pos);
    const DirectX::XMFLOAT2& GetPosition() const;

    void SetRadius(float r);
    float GetRadius() const;

    // 円同士の当たり判定
    bool Intersects(const Collision& other) const;

    // 静的ユーティリティ
    static bool Intersects(const Collision& a, const Collision& b);

private:
    DirectX::XMFLOAT2 m_pos{ 0.0f, 0.0f };
    float m_radius = 0.0f;
};
