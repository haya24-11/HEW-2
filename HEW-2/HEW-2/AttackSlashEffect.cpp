#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"

#include "GamePlay.h"
#include "EnemySpawner.h"
#include "Enemy.h"

#include <cmath>
#include <Windows.h>
#include <iostream> // ✅ std::cout

using namespace DirectX::SimpleMath;

// 旧呼び出し互換（damage無し）
AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir)
    : AttackSlashEffect(scene, owner, dir, /*facingRight*/ true, /*damage*/ 0)
{
}

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir, bool facingRight, int damage)
{
    std::cout << "[SlashEffect] Create dmg=" << damage << "\n";

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
    m_object->Init("asset/Texture/slash_effect.png", 1, 1);
    m_object->SetSize(300.0f, 300.0f, 0.0f);
    m_object->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    m_object->SetUI(false);

    // =========================
    // 初期位置：所有者の前方に配置
    // =========================
    Vector3 p = owner ? owner->GetPos() : Vector3(0, 0, 0);
    Vector2 dirVec = AttackDirToVector(m_dir);

    const float kSlashOffset = 70.0f;
    p.x += dirVec.x * kSlashOffset;
    p.y += dirVec.y * kSlashOffset;
    p.y += -20.0f;

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

    // ✅ パラメータ確認
    std::cout << "[SlashEffect] life=" << m_lifeTime
        << " delay=" << m_delay
        << " hitTime=" << m_hitActiveTime
        << " hitRadius=" << m_hitRadius
        << " dmg=" << m_damage << "\n";
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

        const float kSlashOffset = 70.0f;
        p.x += dirVec.x * kSlashOffset;
        p.y += dirVec.y * kSlashOffset;
        p.y += -20.0f;

        m_object->SetPos(p.x, p.y, p.z);
    }

    // 進行率
    const float t = (m_lifeTime > 0.0f) ? (m_timer / m_lifeTime) : 1.0f;

    // =========================
    // 当たり判定が有効な時間
    // =========================
    const float hitStart = m_delay;
    const float hitEnd = m_delay + m_hitActiveTime;
    const bool hitActive = (m_timer >= hitStart && m_timer <= hitEnd);

    m_object->SetCollisionRadius(hitActive ? m_hitRadius : 0.0f);

    // ✅ 0.2秒마다 상태 로그 (안 뜨면 Update 자체가 안 도는 거)
    {
        static float s_dbg = 0.0f;
        s_dbg += dt;
        if (s_dbg > 0.2f)
        {
            s_dbg = 0.0f;
            std::cout << "[SlashTick] time=" << m_timer
                << " active=" << (hitActive ? 1 : 0)
                << " rad=" << (hitActive ? m_hitRadius : 0.0f)
                << " dmg=" << m_damage << "\n";
        }
    }

    if (t >= 1.0f)
    {
        m_object->SetCollisionRadius(0.0f);
        m_dead = true;
        return;
    }

    // 表示（フェード）
    float alpha = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
    if (m_timer < m_delay) alpha = 0.0f;
    m_object->SetColor(1, 1, 1, alpha);

    // 拡大
    float scale = 0.6f + t * 0.6f;
    m_object->SetSize(250.0f * scale, 90.0f * scale, 0.0f);

    // =========================
    // 敵に当たったらダメージ
    // =========================
    if (hitActive && m_gameplay && m_damage > 0)
    {
        const auto& enemies = m_gameplay->GetSpawner().GetEnemies();

        // ✅ 自前の円判定（CheckCollision を信用しない）
        const auto ap3 = m_object->GetPos();
        const Vector2 aPos(ap3.x, ap3.y);
        const float aR = m_object->GetCollisionRadius(); // = m_hitRadius (hitActive中)

        for (const auto& e : enemies)
        {
            if (!e) continue;
            if (!e->IsAlive()) continue;

            Object* enemyObj = e->GetObject();
            if (!enemyObj) continue;

            if (m_hitOnce.find(e.get()) != m_hitOnce.end()) continue;

            const auto ep3 = enemyObj->GetPos();
            const Vector2 ePos(ep3.x, ep3.y);
            const float eR = enemyObj->GetCollisionRadius();

            const Vector2 d = (ePos - aPos);
            const float rr = (aR + eR);
            const bool hit = (d.LengthSquared() <= rr * rr);

            if (hit)
            {
                std::cout << "[SlashHit] hit! dmg=" << m_damage << " eR=" << eR << "\n";
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