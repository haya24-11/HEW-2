#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"

#include "GamePlay.h"       // ✅ 必要
#include "EnemySpawner.h"   // ✅ 必要
#include "Enemy.h"          // ✅ 必要

#include <cmath>

using namespace DirectX::SimpleMath;

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, bool facingRight, int damage)
{
    m_scene = scene;
    m_damage = damage;

    // ✅ GamePlay取得（敵リストにアクセスするため）
    m_gameplay = dynamic_cast<GamePlay*>(scene);

    m_object = scene->AddObject();
    if (!m_object)
    {
        m_dead = true;
        return;
    }

    // =========================
    // 見た目（斬撃）
    // =========================
    m_object->Init("asset/Texture/slash_effect.png", 1, 1);
    m_object->SetUI(false);

    // 初期サイズ（Updateで拡大していく）
    m_object->SetSize(120.0f, 120.0f, 0.0f);

    // 初期は透明
    m_object->SetColor(1, 1, 1, 0);

    // =========================
    // 位置：プレイヤー前方に出す
    // =========================
    auto p = owner->GetPos();
    const float offsetX = 80.0f;
    const float offsetY = -20.0f;

    m_object->SetPos(
        p.x + (facingRight ? offsetX : -offsetX),
        p.y + offsetY,
        p.z
    );

    // 向き（左なら回転）
    m_object->SetAngle(facingRight ? 0.0f : 180.0f);

    // ✅ 当たり判定（開始時はOFFにして、判定時間だけON）
    m_object->SetCollisionRadius(0.0f);
}

AttackSlashEffect::~AttackSlashEffect()
{
}

void AttackSlashEffect::Update(float dt)
{
    if (m_dead) return;
    if (!m_object) return;

    m_timer += dt;

    // 進行率
    const float t = (m_lifeTime > 0.0f) ? (m_timer / m_lifeTime) : 1.0f;

    if (t >= 1.0f)
    {
        // 終了：判定OFF→消滅
        m_object->SetCollisionRadius(0.0f);
        m_dead = true;
        return;
    }

    // =========================
    // 表示（フェード）
    // =========================
    float alpha = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
    if (m_timer < m_delay) alpha = 0.0f; // 遅延中は見せない
    m_object->SetColor(1, 1, 1, alpha);

    // 拡大
    float scale = 0.6f + t * 0.6f;
    m_object->SetSize(120.0f * scale, 120.0f * scale, 0.0f);

    // =========================
    // ✅ 当たり判定の有効時間
    // =========================
    const float hitStart = m_delay;
    const float hitEnd = m_delay + m_hitActiveTime;
    const bool hitActive = (m_timer >= hitStart && m_timer <= hitEnd);

    m_object->SetCollisionRadius(hitActive ? m_hitRadius : 0.0f);

    // =========================
    // ✅ 敵に当たったらダメージ（プレイヤー攻撃力そのまま）
    // =========================
    if (hitActive && m_gameplay && m_damage > 0)
    {
        const auto& enemies = m_gameplay->GetSpawner().GetEnemies();

        for (const auto& e : enemies)
        {
            if (!e) continue;
            if (!e->IsAlive()) continue;

            Object* enemyObj = e->GetObject();
            if (!enemyObj) continue;

            // 同じ敵はこの攻撃で1回だけ
            if (m_hitOnce.find(e.get()) != m_hitOnce.end()) continue;

            if (m_object->CheckCollision(*enemyObj))
            {
                e->TakeDamage(m_damage);
                m_hitOnce.insert(e.get());
            }
        }
    }
}

void AttackSlashEffect::Uninit()
{
    if (m_scene && m_object)
    {
        m_scene->RemoveObject(m_object);
        m_object = nullptr;
    }
}