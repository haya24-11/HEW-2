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

        // ノックバック中は歩行扱いにしない
        m_animator.Update(deltaTime);
        return;
    }

//==============================
// Chase (AI) 追加
//==============================
    if (!chaseEnabled || !m_target)
        return;

    const auto ePos = m_object->GetPos();
    const auto tPos = m_target->GetPos();

    DirectX::SimpleMath::Vector2 dir(tPos.x - ePos.x, tPos.y - ePos.y);

    bool isMoving = true;

    // 近すぎると止まる
    if (chaseStopDistance > 0.0f)
    {
        const float distSq = dir.LengthSquared();
        const float stopSq = chaseStopDistance * chaseStopDistance;
        if (distSq <= stopSq)
        {
            isMoving = false;
        }
    }

   // ==============================
   // 向き更新
   // ==============================
    if (dir.x > 0.0f)      m_facingRight = true;
    else if (dir.x < 0.0f) m_facingRight = false;

    // ==============================
    // 移動
    // ==============================
    if (isMoving)
    {
        // 移動前の位置保存
        const auto old = m_object->GetPos();

        Move(dir, deltaTime);
        // 重なると元の位置に戻る（壁のように通れないように）
        if (m_object->CheckCollision(*m_target))
        {
            m_object->SetPos(old.x, old.y, old.z);
            isMoving = false;
        }
    }

    // ==============================
    // ★ 移動アニメ制御（ここが本体）
    // ==============================
    if (isMoving)
    {
        if (!m_isWalking)
        {
            m_isWalking = true;
            ApplyWalkVisual();      // ★ 子クラス依存
            m_animator.Play(m_walkAnim);
        }
    }
    else
    {
        // ★ 止まったら歩行終了
        m_isWalking = false;
        // フレーム0固定（Animatorを止める）
        // → Update を呼ばなければOK
    }

    // ==============================
    // 向き反映
    // （enemy walk 原画は左向き想定）
    // ==============================
    bool flipX = (m_textureRightFacing != m_facingRight);
    m_object->SetFlipX(flipX);

    // ==============================
    // アニメ更新（歩いてる時だけ）
    // ==============================
    if (m_isWalking)
    {
        m_animator.Update(deltaTime);
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