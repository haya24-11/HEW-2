#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"

using namespace DirectX::SimpleMath;

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, bool facingRight)
{
    m_object = scene->AddObject();
    if (!m_object)
    {
        m_dead = true;
        return;
    }

    // ▼ 確実に見える設定（デバッグ用）
    m_object->Init("asset/Texture/slash_effect.png", 1, 1);
    m_object->SetSize(300.0f, 300.0f, 0.0f);
    m_object->SetPos(0.0f, 0.0f, 0.0f);     // 画面中央固定
    m_object->SetColor(1.0f, 0.0f, 0.0f, 1.0f); // 真っ赤
    m_object->SetUI(false);                 // UI扱いを明示的にオフ

    auto p = owner->GetPos();
    m_object->SetPos(
        p.x + (facingRight ? 80.0f : -80.0f),
        p.y,
        p.z
    );

    m_object->SetFlipX(!facingRight);
}

AttackSlashEffect::~AttackSlashEffect()
{

}

void AttackSlashEffect::Update(float dt)
{
    if (m_dead) return;
    if (!m_object) return;

    m_timer += dt;
    float t = m_timer / m_lifeTime;

    if (t >= 1.0f)
    {
        m_dead = true;
        return;
    }

    float scale = 0.6f + t * 0.6f;
    m_object->SetSize(120.0f * scale, 120.0f * scale, 0.0f);

    float alpha = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
    m_object->SetColor(1, 1, 1, alpha);
}


void AttackSlashEffect::Uninit()
{
    if (m_object && m_scene)
    {
        m_scene->RemoveObject(m_object);
        m_object = nullptr;
    }
}