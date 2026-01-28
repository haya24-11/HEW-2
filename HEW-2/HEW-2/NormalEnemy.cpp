#include "NormalEnemy.h"

NormalEnemy::NormalEnemy()
{
	hp = 30;
	power = 5;
	isBoss = false;

	moveSpeed = 7.0f;
	chaseStopDistance = 300.0f;

	SetupAnimation();
}

void NormalEnemy::SetupAnimation()
{
    // 歩行アニメ
    m_walkAnim = { 0, 31, 0.1f, true };

    // 原画は左向き
    m_textureRightFacing = false;
}

void NormalEnemy::ApplyWalkVisual()
{
    m_object->SetTexture("asset/Texture/enemy_slime.png");
    m_object->SetSpriteSheet(8, 4);
}

void NormalEnemy::Attack()
{

}
