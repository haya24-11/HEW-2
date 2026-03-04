#include "Chara.h"
#include <algorithm>

void Chara::Update(float /*deltaTime*/)
{

}

void Chara::Move(const DirectX::SimpleMath::Vector2& direction, float deltaTime)
{
    if (!m_object) return;
    if (direction.LengthSquared() == 0.0f) return;

    /*
        正規化する理由
        ・斜め移動(W+Aなど)が速くならないようにするため
    */
    DirectX::SimpleMath::Vector2 dir = direction;
    dir.Normalize();

    auto p = m_object->GetPos();

    // 移動量 = 方向 × 速度 × 経過時間
    p.x += dir.x * moveSpeed * deltaTime;
    p.y += dir.y * moveSpeed * deltaTime;

    m_object->SetPos(p.x, p.y, p.z);
}

void Chara::TakeDamage(int dmg)
{
    if (dmg <= 0) return;

    hp -= dmg;
    if (hp <= 0)
    {
        hp = 0;
        OnHpZero(); // ★ここで派生の処理が呼ばれる（Enemyの遅延死など）
    }
}



void Chara::OnHpZero()
{

}
