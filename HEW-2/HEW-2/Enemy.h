#pragma once
#include "Chara.h"

/*
    Enemy
    =====
    ・敵キャラ共通処理
    ・被ダメージ
    ・ノックバック
*/

class Enemy : public Chara
{
public:
    Enemy();

    void Update(float deltaTime)override;
    void Attack() override;

    void TakeDamage(int damage);
    void KnockBack(const DirectX::SimpleMath::Vector2& force);

protected:
    bool isBoss = false;
};