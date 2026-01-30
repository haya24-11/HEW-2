//#include "Boss.h"
//
//Boss::Boss()
//{
//    hp = 300;
//    power = 20;
//    isBoss = true;
//}

#include "Boss.h"

Boss::Boss()
{
    isBoss = true;

    // ボスはノックバック後に死なない
    EnableDeathAfterKnockback(false);

    // インパクトダメージ大きめ
    SetImpactDamage(30);
}

Enemy::SpawnConfig Boss::GetSpawnConfig() const
{
    SpawnConfig cfg;

    cfg.texture = "asset/Texture/BossEnemy.png";
    cfg.sizeX = 300.0f;
    cfg.sizeY = 300.0f;
    cfg.collisionRadius = 140.0f;

    cfg.hp = 1000;
    cfg.power = 20;

    cfg.minDist = 600.0f;
    cfg.maxDist = 800.0f;
    cfg.interval = 9999.0f; // 再スポーンさせない
    cfg.maxAlive = 1;
    cfg.weight = 0.0f;      // 通常抽選には出さない

    cfg.dieDelay = 2.0f;
    cfg.disappearDelay = 3.0f;

    cfg.useAnim = true;
    cfg.anim = { 0, 8, 0.15f, true };

    return cfg;
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
    // 例：常に大きく表示
    if (auto* obj = GetObject())
        obj->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
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
