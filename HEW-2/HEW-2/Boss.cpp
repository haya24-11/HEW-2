
#include "Boss.h"

Boss::Boss()
{
    hp = 50;
    power = 5;

    isBoss = true;

    moveSpeed = 5.0f;
    // ボスはノックバック後に死なない
    EnableDeathAfterKnockback(false);


    SetupAnimation();
    // インパクトダメージ大きめ
    SetImpactDamage(30);
}

void Boss::SetupAnimation()
{
    m_idleAnim = { 0, 4, 0.3f, true };
    m_walkAnim = { 4, 4, 0.15f, true };
    m_hitAnim = { 8, 2, 0.08f, false };
    m_dieAnim = { 10, 12, 0.25f, false };
}

void Boss::ApplyWalkVisual()
{
    m_object->SetTexture("asset/Texture/BossWalk.png");
    m_object->SetSpriteSheet(8, 4);
}

void Boss::ApplyIdleVisual()
{
    ApplyWalkVisual();
}

void Boss::ApplyHitVisual()
{
    // 被弾時に赤く
    if (auto* obj = GetObject())
        obj->SetColor(1.0f, 0.5f, 0.5f, 1.0f);
}

void Boss::ApplyDieVisual()
{
    // 死亡時は少し暗く
    if (auto* obj = GetObject())
        obj->SetColor(0.6f, 0.6f, 0.6f, 1.0f);
}

void Boss::Attack()
{
}

void Boss::OnDamaged(int damage)
{
    // ボス専用：一定以下のダメージ無効とかも可能
    Enemy::OnDamaged(damage);
}


