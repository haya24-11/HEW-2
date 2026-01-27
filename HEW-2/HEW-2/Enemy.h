#pragma once
#include "Chara.h"
#include "Object.h"
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

    /*
        KnockBack
        ----------
        ・Mode 側から呼ばれる
        ・AMode のときだけ呼ばれる想定
        ・force : ノックバックの初速（方向×強さ）：：：：：
    */
    //target設定(player)
    void SetTarget(Object* target) { m_target = target; }
    void SetChaseStopDistance(float d) { chaseStopDistance = d; }
    float GetChaseStopDistance() const { return chaseStopDistance; }
    //追跡ON OFF
    void SetChaseEnabled(bool enabled) { chaseEnabled = enabled; }
    bool IsChaseEnabled() const { return chaseEnabled; }

    void KnockBack(const DirectX::SimpleMath::Vector2& force);

protected:
    bool isBoss = false;


    Object* m_target = nullptr;        // 追跡対象
    float chaseStopDistance = 0.0f;    // 0なら密着まで追跡(衝突時に戻す)
    bool chaseEnabled = true;

    // ノックバック用
    DirectX::SimpleMath::Vector2 knockBackVelocity{ 0.0f,0.0f };
    float knockBackTimer = 0.0f;

    // ノックバック継続時間（秒）
    float knockBackDuration = 0.15f;
};