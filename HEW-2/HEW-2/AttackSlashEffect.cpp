#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"

#include "GamePlay.h"
#include "EnemySpawner.h"
#include "Enemy.h"

#include <cmath>
#include <Windows.h> // OutputDebugStringA

using namespace DirectX::SimpleMath;

// 旧呼び出し互換（damage無し）
AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir)
    : AttackSlashEffect(scene, owner, dir, /*facingRight*/ true, /*damage*/ 0)
{
}

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir, bool facingRight, int damage)
{
    OutputDebugStringA("SlashEffect Create\n");

    m_scene = scene;
    m_owner = owner;
    m_dir = dir;
    m_damage = damage;

    // GamePlay取得（敵リストにアクセスするため）
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
     // ▼ 確実に見える設定（デバッグ用）
    m_object->Init("asset/Texture/slash_effect.png", 1, 1);
    m_object->SetSize(300.0f, 300.0f, 0.0f);
    m_object->SetColor(1.0f, 1.0f, 1.0f, 1.0f); // 真っ赤
    m_object->SetUI(false);

    // =========================
    // 初期位置：所有者の前方に配置
    // =========================
    Vector3 p = owner ? owner->GetPos() : Vector3(0, 0, 0);
    Vector2 dirVec = AttackDirToVector(m_dir);

    p.x += dirVec.x * 120.0f;
    p.y += dirVec.y * 120.0f;
    p.y += -20.0f; // 下に少し

    m_object->SetPos(p.x, p.y, p.z);

    // 攻撃方向に応じて角度
    switch (m_dir)
    {
    case AttackDir::Right:     m_object->SetAngle(0.0f);    break;
    case AttackDir::Left:      m_object->SetAngle(180.0f);  break;
    case AttackDir::Up:        m_object->SetAngle(90.0f);   break;
    case AttackDir::Down:      m_object->SetAngle(-90.0f);  break;
    case AttackDir::UpRight:   m_object->SetAngle(45.0f);   break;
    case AttackDir::UpLeft:    m_object->SetAngle(135.0f);  break;
    case AttackDir::DownRight: m_object->SetAngle(-45.0f);  break;
    case AttackDir::DownLeft:  m_object->SetAngle(-135.0f); break;
    default:                   m_object->SetAngle(0.0f);    break;
    }

    // 当たり判定（開始時OFF）
    m_object->SetCollisionRadius(0.0f);
}

AttackSlashEffect::~AttackSlashEffect()
{
}

void AttackSlashEffect::Update(float dt)
{
    if (m_dead || !m_object) return;

    m_timer += dt;

    // ★プレイヤー追従（常に前方へ）
    if (m_owner)
    {
        Vector3 p = m_owner->GetPos();
        Vector2 dirVec = AttackDirToVector(m_dir);

        p.x += dirVec.x * 120.0f;
        p.y += dirVec.y * 120.0f;
        p.y += -20.0f;

        m_object->SetPos(p.x, p.y, p.z);
    }

    // 進行率
    const float t = (m_lifeTime > 0.0f) ? (m_timer / m_lifeTime) : 1.0f;

    if (t >= 1.0f)
    {
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
    m_object->SetSize(250.0f * scale, 90.0f * scale, 0.0f);

    // =========================
    // 当たり判定が有効な時間
    // =========================
    const float hitStart = m_delay;
    const float hitEnd = m_delay + m_hitActiveTime;
    const bool hitActive = (m_timer >= hitStart && m_timer <= hitEnd);

    m_object->SetCollisionRadius(hitActive ? m_hitRadius : 0.0f);

    // =========================
    // 敵に当たったらダメージ（攻撃力そのまま）
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