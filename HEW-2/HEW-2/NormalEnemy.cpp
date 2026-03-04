#include "NormalEnemy.h"

NormalEnemy::NormalEnemy()
{
	isBoss = false;
    const auto cfg = GetSpawnConfig();
    hp = cfg.hp;
    power = cfg.power;
	moveSpeed = 7.0f;

	SetupAnimation();
}

void NormalEnemy::SetupAnimation()
{
    // 歩行アニメ
    m_walkAnim = { 0, 31, 0.1f, true };
    m_idleAnim = { 0, 31, 0.1f, true };
    m_hitAnim  = { 0, 31, 0.1f, true };
    m_dieAnim  = { 0, 31, 0.1f, true };
    // 原画は左向き
    m_textureRightFacing = false;
}
/**/
void NormalEnemy::ApplyWalkVisual()
{
    m_object->SetTexture("asset/Texture/enemy_slime.png");
    m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::Attack()
{

}
//モーション追加する時利用してください。

void NormalEnemy::ApplyIdleVisual()
{
    // 基本は Walk と同じシートを使用
    m_object->SetTexture("asset/Texture/enemy_slime.png");
    m_object->SetSpriteSheet(8, 4);

    // もし Idle だけ別テクスチャ/別シートを使いたい場合は、ここで差し替える
    // m_object->SetTexture("asset/Texture/NormalEnemy_idle.png");
    // m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::ApplyHitVisual()
{
    m_object->SetTexture("asset/Texture/enemy_slime_down.png");
    m_object->SetSpriteSheet(8, 4);
    //m_object->SetTexture("asset/Texture/enemy_slime_down.png");
    //m_object->SetSpriteSheet(8, 4);
}


void NormalEnemy::ApplyDieVisual()
{
    m_object->SetTexture("asset/Texture/enemy_slime_down.png");
    m_object->SetSpriteSheet(8, 4);
    //m_object->SetTexture("asset/Texture/enemy_slime_down.png");
    //m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::OnDamaged(int damage)
{
    // ✅ 被弾モーション再生（Enemy 側の PlayHit() を使用）
    PlayHit();

    // （任意）追加処理：ノックバック強化、色変更、効果音、エフェクト等
    // 例：damage が大きい時はより強く吹き飛ばす、など
    (void)damage;
}

