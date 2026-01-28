#include "SlimeEnemy.h"

SlimeEnemy::SlimeEnemy()
{
    moveSpeed = 40.0f;
    SetupAnimation();
}

void SlimeEnemy::SetupAnimation()
{
    // 原画は左向き
    m_textureRightFacing = false;   // 左向き原画

    // 歩行アニメ定義
    m_walkAnim = { 0, 31, 0.1f, true };
}

void SlimeEnemy::ApplyWalkVisual()
{
    m_object->SetTexture("asset/Texture/enemy_slime.png");
    m_object->SetSpriteSheet(8, 4);
}