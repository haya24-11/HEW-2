#include "AttackSlashEffect.h"

using namespace DirectX::SimpleMath;

AttackSlashEffect::AttackSlashEffect(Object* owner, bool facingRight)
{
    // エフェクト用 Object を生成
    m_object = new Object();
    m_object->SetTexture("asset/Texture/slash_effect.png"); // 三日月
    m_object->SetSpriteSheet(1, 1);

    // プレイヤー位置取得
    auto p = owner->GetPos();

    // 前方オフセット
    float offsetX = facingRight ? 60.0f : -60.0f;
    p.x += offsetX;

    m_object->SetPos(p.x, p.y, p.z);
    m_object->SetSize(120.0f, 120.0f, 0.0f);

    // 原画が右向き想定
    m_object->SetFlipX(!facingRight);

    // 少し角度をつけると斬撃っぽい
    m_object->SetAngle(facingRight ? -20.0f : 20.0f);
}

AttackSlashEffect::~AttackSlashEffect()
{
    delete m_object;
}

void AttackSlashEffect::Update(float deltaTime)
{
    if (m_dead) return;

    m_timer += deltaTime;

    float t = m_timer / m_lifeTime;
    if (t >= 1.0f)
    {
        m_dead = true;
        return;
    }

    // ===== 擬似アニメーション =====

    // スケール（最初小さく → 最大）
    float scale = 0.6f + t * 0.6f;
    m_object->SetSize(120.0f * scale, 120.0f * scale, 0.0f);

    // フェードアウト（後半）
    float alpha = (t < 0.5f) ? 1.0f : (1.0f - (t - 0.5f) * 2.0f);
    m_object->SetColor(1.0f, 1.0f, 1.0f, alpha);
}
