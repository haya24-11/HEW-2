#include "Enemy.h"

Enemy::Enemy() {}

void Enemy::OnSpawned()
{
    if (!m_object) return;
    // 派生クラス側のコンストラクタで SetupAnimation() を呼ぶことが多いが、
    // 万が一呼び忘れていた場合に備えて、ここで呼んでもよい（必要ならコメント解除）
    // SetupAnimation();

    PlayIdle();
}

void Enemy::OnDamaged(int /*damage*/)
{
    // 基本動作：被弾モーションを再生
    PlayHit();
}

void Enemy::PlayIdle()
{
    if (m_animState == AnimState::Idle) return;
    m_animState = AnimState::Idle;
    m_isWalking = false;

    ApplyIdleVisual();
    m_animator.Play(m_idleAnim);
}

void Enemy::PlayWalk()
{
    if (m_animState == AnimState::Walk) return;
    m_animState = AnimState::Walk;
    m_isWalking = true;

    ApplyWalkVisual();
    m_animator.Play(m_walkAnim);
}

void Enemy::PlayHit()
{
    m_animState = AnimState::Hit;
    m_isWalking = false;

    ApplyHitVisual();
    m_animator.Play(m_hitAnim);
}

void Enemy::Update(float deltaTime)
{
    Chara::Update(deltaTime);

    if (!m_object) return;

    // -------------------------
    // ノックバック中
    // -------------------------
    if (knockBackTimer > 0.0f)
    {
        position = m_object->GetPos();
        position.x += knockBackVelocity.x * deltaTime;
        position.y += knockBackVelocity.y * deltaTime;
        m_object->SetPos(position.x, position.y, position.z);

        knockBackTimer -= deltaTime;
        if (knockBackTimer <= 0.0f)
        {
            knockBackVelocity = { 0.0f, 0.0f };
            knockBackTimer = 0.0f;
        }

        // 被弾/移動アニメを含め、再生中のアニメは継続して更新する
        m_animator.Update(deltaTime);
        return;
    }

    // -------------------------
    // 追跡（Chase）計算
    // -------------------------
    bool isMoving = false;
    DirectX::SimpleMath::Vector2 dir(0.0f, 0.0f);

    if (chaseEnabled && m_target)
    {
        const auto ePos = m_object->GetPos();
        const auto tPos = m_target->GetPos();
        dir = { tPos.x - ePos.x, tPos.y - ePos.y };

        isMoving = true;

        if (chaseStopDistance > 0.0f)
        {
            const float distSq = dir.LengthSquared();
            const float stopSq = chaseStopDistance * chaseStopDistance;
            if (distSq <= stopSq) isMoving = false;
        }

        // 向きの更新
        if (dir.x > 0.0f)      m_facingRight = true;
        else if (dir.x < 0.0f) m_facingRight = false;

        // 移動
        if (isMoving)
        {
            const auto old = m_object->GetPos();

            Move(dir, deltaTime);

            if (m_object->CheckCollision(*m_target))
            {
                // 対象（ターゲット）に当たったら移動前の座標へ戻す
                m_object->SetPos(old.x, old.y, old.z);
                isMoving = false;
            }
        }
    }

    // -------------------------
    // アニメ制御（Hit優先）
    // -------------------------
    if (m_animState == AnimState::Hit)
    {
        m_animator.Update(deltaTime);

        if (m_animator.IsFinished())
        {
            // 被弾が終わったら、現在の移動状態に応じて復帰
            if (isMoving) PlayWalk();
            else          PlayIdle();
        }
    }
    else
    {
        if (isMoving) PlayWalk();
        else          PlayIdle();

        m_animator.Update(deltaTime);
    }

    // 向き反映
    bool flipX = (m_textureRightFacing != m_facingRight);
    m_object->SetFlipX(flipX);
}

void Enemy::TakeDamage(int damage)
{
    hp -= damage;
    OnDamaged(damage); // 派生クラス側のcppで被弾モーションを管理できる
}

void Enemy::KnockBack(const DirectX::SimpleMath::Vector2& force)
{
    if (force.LengthSquared() == 0.0f) return;
    if (isBoss) return;

    knockBackVelocity = force;
    knockBackTimer = knockBackDuration;
}
