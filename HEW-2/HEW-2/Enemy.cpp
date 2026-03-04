#include "Enemy.h"
#include <cmath>
#include <algorithm>
#include <cmath> // fmodf

Enemy::Enemy() {}

void Enemy::OnSpawned()
{
    auto cfg = GetSpawnConfig();
    SetDeathDelay(2.0f);   
    SetDisappearDelay(0.0f);  
    PlayIdle();
}


void Enemy::OnDamaged(int /*damage*/)
{
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
    m_animState = AnimState::Hit; return;
    m_isWalking = false;

    ApplyHitVisual();
    m_animator.Play(m_hitAnim);
}

void Enemy::PlayDie()
{
    if (m_animState == AnimState::Die)return;

    m_animState = AnimState::Die;
    m_isWalking = false;

    ApplyDieVisual();
    m_animator.Play(m_dieAnim);
}
void Enemy::BeginDeath()
{
    // すでに死亡演出中なら二重開始しない
    if (m_isDying) return;

    m_isDying = true;

    // ✅ 死亡モーションを見せる時間をセット
    m_dieTimer = m_dieDelay;

    // ✅ 死亡後にさらに残す時間（任意）を必ず初期化
    m_disappearTimer = m_disappearDelay;

    // 追跡停止（必要なら）
    chaseEnabled = false;

    // 死亡アニメ開始
    PlayDie();
}

void Enemy::OnHpZero()
{
    // HPを0に固定
    hp = 0;

    // ✅ 強攻撃など「ノックバックが終わってから死ぬ」モードなら予約だけ入れる
    if (m_deathAfterKnockback)
    {
        m_pendingDeath = true;
        return; // ここでは死亡演出を開始しない
    }

    // ノックバックしていない（または終了している）なら死亡演出を開始
    if (knockBackTimer <= 0.0f)
    {
        // 念のためノックバック状態を解除
        StopKnockBack();

        // ※ この時点で m_pendingDeath は通常 false のはず（上で return しているため）
        // ここで死なせたいなら BeginDeath() を直接呼ぶのが正しい
        if (m_pendingDeath)
        {
            BeginDeath(); // （必要なら DieNow() で即消し）
            return;
        }
    }
}

// ✅ 最終的に画面から消す（非表示＋当たり判定無効）
void Enemy::DieNow()
{
    m_isDying = false;
    isAlive = false;

    if (m_object)
    {
        // 当たり判定を無効化
        m_object->SetCollisionRadius(0.0f);

        // 完全透明にする
        m_object->SetColor(1, 1, 1, 0.0f);

        // 画面外へ移動（安全策）
        auto p = m_object->GetPos();
        m_object->SetPos(999999.0f, 999999.0f, p.z);
    }
}

void Enemy::Update(float deltaTime)
{
    // ✅ ヒットフラッシュ更新
    UpdateHitFlash(deltaTime);

    if (!m_object) return;

    // ✅ インパクト（連打防止）クールダウン更新
    if (m_impactCooldown > 0.0f)
    {
        m_impactCooldown -= deltaTime;
        if (m_impactCooldown < 0.0f) m_impactCooldown = 0.0f;
    }

    // =========================
    // ✅ 死亡中：死亡モーションを見せてから消す
    // =========================
    if (m_isDying)
    {
        // 死亡モーション更新
        m_animator.Update(deltaTime);

        // 1) 死亡モーション保持時間
        if (m_dieTimer > 0.0f)
        {
            m_dieTimer -= deltaTime;
            if (m_dieTimer <= 0.0f) m_dieTimer = 0.0f;
            return;
        }

        // 2) 死亡後の追加保持時間（任意）
        if (m_disappearTimer > 0.0f)
        {
            m_disappearTimer -= deltaTime;
            if (m_disappearTimer <= 0.0f) m_disappearTimer = 0.0f;
            return;
        }

        // 3) タイマー終了 → 最終的に消す
        DieNow();
        return;
    }

    // ✅ 「死亡予約」かつ「もうノックバックしていない」なら死亡演出を開始
    if (m_pendingDeath && !IsKnockBacking())
    {
        BeginDeath();
        m_pendingDeath = false; // ✅ 開始したら予約を消す
        return;
    }

    // -------------------------
    // ✅ ノックバック中
    // -------------------------
    if (knockBackTimer > 0.0f)
    {
        auto pos = m_object->GetPos();
        pos.x += knockBackVelocity.x * deltaTime;
        pos.y += knockBackVelocity.y * deltaTime;
        m_object->SetPos(pos.x, pos.y, pos.z);

        // ✅ 速度がほぼ0なら「止まった扱い」でノックバック終了（任意）
        if (knockBackVelocity.LengthSquared() < 1.0f * 1.0f)
        {
            knockBackVelocity = { 0.0f, 0.0f };
            knockBackTimer = 0.0f;

            // 死亡予約なら死亡演出開始
            if (m_pendingDeath)
            {
                BeginDeath();
                m_pendingDeath = false;
                return;
            }
        }

        knockBackTimer -= deltaTime;

        // ✅ ノックバック終了した瞬間に死亡予約なら → 死亡演出開始（即消しではない）
        if (knockBackTimer <= 0.0f)
        {
            knockBackVelocity = { 0.0f, 0.0f };
            knockBackTimer = 0.0f;

            if (m_pendingDeath)
            {
                BeginDeath();
                m_pendingDeath = false;
                return;
            }
        }

        m_animator.Update(deltaTime);
        return;
    }

    // -------------------------
    // ✅ 追跡（Chase）
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

        if (dir.x > 0.0f)      m_facingRight = true;
        else if (dir.x < 0.0f) m_facingRight = false;

        if (isMoving)
        {
            const auto old = m_object->GetPos();

            Move(dir, deltaTime);

            // プレイヤーにめり込む場合は戻す
            if (m_object->CheckCollision(*m_target))
            {
                m_object->SetPos(old.x, old.y, old.z);
                isMoving = false;
            }
        }
    }

    // -------------------------
    // ✅ アニメ制御（Hit優先）
    // -------------------------
    if (m_animState == AnimState::Hit)
    {
        m_animator.Update(deltaTime);

        if (m_animator.IsFinished())
        {
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

    // ✅ 向き反映
    bool flipX = (m_textureRightFacing != m_facingRight);
    m_object->SetFlipX(flipX);
}


void Enemy::TakeDamage(int damage)
{
    if (!isAlive) return;
    if (damage <= 0) return;
    if (m_isDying) return; // 死亡演出中なら追加ダメージは無視（必要なら外す）

    // 被弾演出
    StartHitFlash(0.15f);
    OnDamaged(damage);

    // ✅ HP減少（HPが0になったら Enemy::OnHpZero() が呼ばれる前提：Chara側の実装次第）
    Chara::TakeDamage(damage);

    // ✅ 今回の攻撃でのみ有効にする（次フレーム以降に持ち越さない）
    m_deathAfterKnockback = false;
}

void Enemy::KnockBack(const DirectX::SimpleMath::Vector2& force)
{
    if (force.LengthSquared() == 0.0f) return;
    if (isBoss) return;
    if (m_isDying) return; // 死亡演出中はノックバックさせない（必要なら外す）

    knockBackVelocity = force;
    knockBackTimer = knockBackDuration;
}


// =========================
// Hit Flash
// =========================
void Enemy::StartHitFlash(float sec)
{
    m_hitFlashDuration = sec;
    m_hitFlashTimer = sec;

    if (m_object)
    {
        m_object->SetColor(1.0f, 0.2f, 0.2f, 1.0f);
    }
}

void Enemy::UpdateHitFlash(float dt)
{
    if (m_hitFlashTimer <= 0.0f) return;

    m_hitFlashTimer -= dt;
    if (!m_object) return;

    const float blinkSpeed = 8.0f; // 点滅速度
    float s = (fmodf(m_hitFlashTimer * blinkSpeed, 1.0f) < 0.5f) ? 1.0f : 0.3f;
    m_object->SetColor(1.0f, 0.2f * s, 0.2f * s, 1.0f);

    if (m_hitFlashTimer <= 0.0f)
    {
        m_object->SetColor(1, 1, 1, 1);
        m_hitFlashTimer = 0.0f;
    }
}

