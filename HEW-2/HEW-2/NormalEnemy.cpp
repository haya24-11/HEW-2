#include "NormalEnemy.h"

NormalEnemy::NormalEnemy()
{
	hp = 30;
	power = 5;
	isBoss = false;

	moveSpeed = 15.0f;
	chaseStopDistance = 300.0f;

	SetupAnimation();
}

void NormalEnemy::SetupAnimation()
{
    // 歩行アニメ
    m_walkAnim = { 0, 31, 0.1f, true };
    m_hitAnim =  { 0, 31, 0.1f, true };
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
//モーション追加sする時利用してください。

void NormalEnemy::ApplyIdleVisual()
{
    // 基本は Walk と同じシートを使用
    ApplyWalkVisual();

    // もし Idle だけ別テクスチャ/別シートを使いたい場合は、ここで差し替える
    // m_object->SetTexture("asset/Texture/NormalEnemy_idle.png");
    // m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::ApplyHitVisual()
{
    // 基本は Walk と同じスプライトシートを使用
    ApplyWalkVisual();

    // ✅ 被弾時だけ別テクスチャ/別シートを使いたい場合は、ここでのみ差し替えればOK
    // m_object->SetTexture("asset/Texture/enemy_slime_hit.png");
    // m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::OnDamaged(int damage)
{
    // ✅ 被弾モーション再生（Enemy 側の PlayHit() を使用）
    PlayHit();

    // （任意）追加処理：ノックバック強化、色変更、効果音、エフェクト等
    // 例：damage が大きい時はより強く吹き飛ばす、など
    (void)damage;
}
