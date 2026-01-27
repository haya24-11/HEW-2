#include "Enemy.h"

Enemy::Enemy()
{

}

void Enemy::Update(float deltaTime)
{
    // Chara 共通更新（HPチェックなど）
	Chara::Update(deltaTime);
    
    // Object が無ければ何もしない
    if (!m_object)
        return;
    /*
       ノックバック中の処理
       --------------------
       ・残り時間がある間だけ押し出す
       ・時間経過で自然に止まる
   */
    if (knockBackTimer > 0.0f)
    {
        // 現在座標取得
        position = m_object->GetPos();

        // ノックバック移動（2D）
        position.x += knockBackVelocity.x * deltaTime;
        position.y += knockBackVelocity.y * deltaTime;

        // Object に反映
        m_object->SetPos(position.x, position.y, position.z);

        knockBackTimer -= deltaTime;

        // 時間切れ
        if (knockBackTimer <= 0.0f)
        {
            knockBackVelocity = { 0.0f, 0.0f };
            knockBackTimer = 0.0f;
        }
    }
}

void Enemy::Attack()
{
    // AI / 行動決定は別担当
}

void Enemy::TakeDamage(int damage)
{
    hp -= damage;
}

void Enemy::KnockBack(const DirectX::SimpleMath::Vector2& force)
{
    /*
        Mode 側で
        ・AMode のときだけ呼ばれる
        ・他モードでは呼ばれない or force が 0
        という前提
    */
    if (force.LengthSquared() == 0.0f)
        return;
    // Boss だけノックバック無効
    if (isBoss) return;

    knockBackVelocity = force;
    knockBackTimer = knockBackDuration;
}