//==============================
// AttackSlashEffect.cpp
//==============================
#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"
#include "GamePlay.h"
#include "EnemySpawner.h"
#include "Enemy.h"

#include <cmath>
#include <Windows.h>

using namespace DirectX::SimpleMath;

// 旧呼び出し互換（damage無し）
AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir)
    : AttackSlashEffect(scene, owner, dir, /*facingRight*/ true, /*damage*/ 0)
{
}

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, AttackDir dir, bool /*facingRight*/, int damage)
{
    m_scene = scene;
    m_owner = owner;
    m_dir = dir;
    m_damage = damage;

    // GamePlay取得（敵リストにアクセスするため）
    m_gameplay = dynamic_cast<GamePlay*>(scene);

    // SceneにObjectを追加（Scene側が所有）
    if (!m_scene)
    {
        m_dead = true;
        return;
    }

    m_object = m_scene->AddObject();
    if (!m_object)
    {
        m_dead = true;
        return;
    }

    // 見た目（斬撃）
    m_object->Init("asset/Texture/slash_effect.png", 1, 1);
    m_object->SetSize(300.0f, 300.0f, 0.0f);
    m_object->SetColor(1.0f, 1.0f, 1.0f, 0.0f);
    m_object->SetUI(false);

    // 当たり判定（開始時OFF）
    m_object->SetCollisionRadius(0.0f);

    // 角度
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
}

AttackSlashEffect::~AttackSlashEffect()
{
    Uninit();
}

void AttackSlashEffect::Update(float dt)
{
    // すでに死亡フラグが立っているか、描画オブジェクトが消えていたら終了
    if (m_dead || !m_object) { m_dead = true; return; }

    m_timer += dt;

    // 🔴 安全策：owner(プレイヤー)が消えている、もしくはシーンが破棄され始めているなら即座に終了
    if (!m_owner || !m_scene || !m_gameplay)
    {
        Uninit();
        m_dead = true;
        return;
    }

    // プレイヤー追従
    {
        Vector3 p = m_owner->GetPos();
        Vector2 dirVec = AttackDirToVector(m_dir);
        const float kSlashOffset = 70.0f;
        p.x += dirVec.x * kSlashOffset;
        p.y += dirVec.y * kSlashOffset;
        p.y += -20.0f;
        m_object->SetPos(p.x, p.y, p.z);
    }

    // 進行率と当たり判定の期間計算
    const float t = (m_lifeTime > 0.0f) ? (m_timer / m_lifeTime) : 1.0f;
    const float hitStart = m_delay;
    const float hitEnd = m_delay + m_hitActiveTime;
    const bool hitActive = (m_timer >= hitStart && m_timer <= hitEnd);

    m_object->SetCollisionRadius(hitActive ? m_hitRadius : 0.0f);

    // 表示フェード・拡大
    float alpha = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
    if (m_timer < m_delay) alpha = 0.0f;
    m_object->SetColor(1, 1, 1, alpha);

    float scale = 0.6f + t * 0.6f;
    m_object->SetSize(250.0f * scale, 90.0f * scale, 0.0f);

    // 寿命終了
    if (t >= 1.0f)
    {
        Uninit();
        m_dead = true;
        return;
    }

    // 🔴 敵への当たり判定（厳重ガード版）
    if (hitActive && m_gameplay && m_damage > 0)
    {
        // Spawnerが生きているかチェック
        auto& spawner = m_gameplay->GetSpawner();
        const auto& enemies = spawner.GetEnemies();

        const auto ap3 = m_object->GetPos();
        const Vector2 aPos(ap3.x, ap3.y);
        const float aR = m_object->GetCollisionRadius();

        for (const auto& e : enemies)
        {
            // 1. ポインタ自体の有効性チェック
            if (!e) continue;

            // 2. 🔴 超重要：不正なメモリアドレス(0xFF...FF)を指していないかチェック
            //    (シーン切り替え時に unique_ptr の中身が壊れるのを防ぐ)
            if (e.get() == (void*)0xFFFFFFFFFFFFFFFF || (size_t)e.get() < 0x10000) continue;

            if (!e->IsAlive()) continue;

            // 3. Enemyが持つ描画Objectのチェック
            Object* enemyObj = e->GetObject();
            if (!enemyObj || enemyObj == (void*)0xFFFFFFFFFFFFFFFF) continue;

            // 4. 重複ヒットチェック
            if (m_hitOnce.find(e.get()) != m_hitOnce.end()) continue;

            const auto ep3 = enemyObj->GetPos();
            const Vector2 ePos(ep3.x, ep3.y);
            const float eR = enemyObj->GetCollisionRadius();

            const Vector2 d = (ePos - aPos);
            const float rr = (aR + eR);

            // 5. 距離判定
            if (d.LengthSquared() <= rr * rr)
            {
                // 🔴 最後に念押しでもう一度チェックしてからダメージ適用
                if (e) {
                    e->TakeDamage(m_damage);
                    m_hitOnce.insert(e.get());
                }
            }
        }
    }
}

void AttackSlashEffect::Uninit()
{
    if (m_scene && m_object)
    {
        m_scene->RemoveObject(m_object);
    }

    m_object = nullptr;
    m_owner = nullptr;
    m_gameplay = nullptr;
    m_scene = nullptr;
    m_hitOnce.clear();
}