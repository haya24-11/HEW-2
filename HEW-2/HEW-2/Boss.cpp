
#include "Boss.h"
#include<iostream>
using namespace std;
Boss::Boss()
{
    isBoss = true;

    moveSpeed = 5.0f;
    EnableDeathAfterKnockback(false);

    // ✅ 念のため：ボスのhp/powerをSpawnConfigの値で確定させる
    const auto cfg = GetSpawnConfig();
    hp = cfg.hp;
    power = cfg.power;

    SetupAnimation();
    SetImpactDamage(5);
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
    std::cout << "[BossDamaged] dmg=" << damage << " hp(before)=" << hp << "\n";
    Enemy::OnDamaged(damage);
    std::cout << "[BossDamaged] hp(after)=" << hp << "\n";
}