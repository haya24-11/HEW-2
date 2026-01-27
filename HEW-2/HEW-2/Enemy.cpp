#include "Enemy.h"

Enemy::Enemy()
{
    moveSpeed = 40.0f;
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
        return;
    }

    //==============================
// Chase (AI) 追加
//==============================
    if (!chaseEnabled) return;
    if (!m_target)     return;

    const auto ePos = m_object->GetPos();
    const auto tPos = m_target->GetPos();

    DirectX::SimpleMath::Vector2 dir(tPos.x - ePos.x, tPos.y - ePos.y);
    // 近すぎると止まる
    if (chaseStopDistance > 0.0f)
    {
        const float distSq = dir.LengthSquared();
        const float stopSq = chaseStopDistance * chaseStopDistance;
        if (distSq <= stopSq) return;
    }
    // 移動前の位置保存
    const auto old = m_object->GetPos();

    Move(dir, deltaTime);
    // 重なると元の位置に戻る（壁のように通れないように）
    if (m_object->CheckCollision(*m_target))
    {
        m_object->SetPos(old.x, old.y, old.z);
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