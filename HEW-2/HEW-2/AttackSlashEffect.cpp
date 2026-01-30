#include "AttackSlashEffect.h"
#include "Scene.h"
#include "Object.h"

using namespace DirectX::SimpleMath;

AttackSlashEffect::AttackSlashEffect(Scene* scene, Object* owner, bool facingRight)
{
    m_scene = scene;

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
    // =========================
    // 描画位置（少し下にずらす）
    // =========================
    const float offsetX = 80.0f;
    const float offsetY = -20.0f;   //  下に描画（マイナスで下）

    m_object->SetPos(
        p.x + (facingRight ? offsetX : -offsetX),
        p.y + offsetY,
        p.z
    );

    // =========================
    // ★ 向きによる角度反転
    // =========================
    // 右向き画像を基準にする
    // 左向きなら180度回転
    if (!facingRight)
    {
        m_object->SetAngle(180.0f);
    }
    else
    {
        m_object->SetAngle(0.0f);
    }
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
    if (m_scene && m_object)
    {
        m_scene->RemoveObject(m_object);
        m_object = nullptr;
    }
}