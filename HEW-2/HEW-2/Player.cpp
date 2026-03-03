#include "Player.h"
#include "Skill.h"

#include <Windows.h>   // GetAsyncKeyState
#include <Xinput.h>    // XInput
#include <cmath>       // fabsf
#include "sound.h"
#include<iostream>
#pragma comment(lib, "Xinput.lib")

namespace SM = DirectX::SimpleMath;

Player::Player()
{
    hp = 50;
    power = 10;

    moveSpeed = 30.0f;

    // アニメーション（開始フレーム / フレーム数 / 1フレーム時間 / ループ）
    m_idleAnim = { 0, 30, 0.15f, true };
    m_walkAnim = { 0,  8, 0.50f, true };
    m_attackLightAnim = { 0, 15, 0.17f, false };
    m_heavyChargeAnim = { 0,  8, 0.20f, false };
    m_heavyStartAnim = { 8, 19, 0.20f, false };

    // 被ダメは m_damagedAnim(横5枚) を使用
}

void Player::Update(float deltaTime)
{
    // 強攻撃後の「被弾アニメ禁止」タイマー更新（GamePlay から参照）
    if (m_noHitAnimTimer > 0.0f)
    {
        m_noHitAnimTimer -= deltaTime;
        if (m_noHitAnimTimer < 0.0f) m_noHitAnimTimer = 0.0f;
    }

    if (m_invincibleTimer > 0.0f)
    {
        m_invincibleTimer -= deltaTime;
        if (m_invincibleTimer < 0.0f) m_invincibleTimer = 0.0f;
    }
    if (m_object)
    {
        if (m_invincibleTimer > 0.0f)
        {
            const float kBlink = 0.08f;               // 点滅の速さ（小さいほど速い）
            const float t = fmodf(m_invincibleTimer, kBlink * 2.0f);
            const float a = (t < kBlink) ? 0.25f : 1.0f;

            m_object->SetColor(1.0f, 1.0f, 1.0f, a); // ✅ キラキラ（透明度）
        }
        else
        {
            m_object->SetColor(1.0f, 1.0f, 1.0f, 1.0f); // ✅ 無敵が終わったら元に戻す
        }
    }
    if (m_hitInvTimer > 0.0f)
    {
        m_hitInvTimer -= deltaTime;
        if (m_hitInvTimer < 0.0f) m_hitInvTimer = 0.0f;
    }

    m_attackInputTriggered = false;

    if (Input::GetKeyTrigger(VK_RETURN))
    {
        m_attackInputTriggered = true;
    }

    // ===== XInput：ボタン取得（トリガー判定用） =====
    XINPUT_STATE pad{};
    WORD buttons = 0;
    if (XInputGetState(0, &pad) == ERROR_SUCCESS)
        buttons = pad.Gamepad.wButtons;

    // 押した瞬間（今押している && 前フレームは押していない）
    auto PadTrigger = [&](WORD mask) -> bool
        {
            return (buttons & mask) && !(m_prevPadButtons & mask);
        };

    // 強攻撃：A / 弱攻撃：B
    const bool attackHeavyPad = PadTrigger(XINPUT_GAMEPAD_A);
    const bool attackLightPad = PadTrigger(XINPUT_GAMEPAD_B);

    // ===== キーボード攻撃 =====
    const bool attackLightKey = Input::GetKeyTrigger(VK_RETURN);
    const bool attackHeavyKey = (GetAsyncKeyState(VK_SHIFT) & 0x8000) && Input::GetKeyTrigger(VK_RETURN);

    // 攻撃入力（キーボード or パッド）
    const bool attackLightInput = attackLightKey || attackLightPad;
    const bool attackHeavyInput = attackHeavyKey || attackHeavyPad;

    // 早期returnが多いので、return直前に必ず前フレームボタンを更新する
    auto CommitPad = [&]()
        {
            m_prevPadButtons = buttons;
        };

    // =========================
    // ✅ 無敵タイマー更新（連続ヒット防止）
    // =========================
    // =========================
// ✅ 無敵タイマー更新（連続ヒット防止）
// ※ どの return よりも前に必ず置く
// =========================

    /* if (m_hitReactCD > 0.0f)
     {
         m_hitReactCD -= deltaTime;
         if (m_hitReactCD < 0.0f) m_hitReactCD = 0.0f;
     }*/
     // =========================
     // ✅ 被ダメージ中：入力無視、アニメ終了でIdleへ戻す
     // =========================
    if (m_state == State::Damaged)
    {
        // エフェクト更新（残っていたら掃除）
        for (auto it = m_attackEffects.begin(); it != m_attackEffects.end(); )
        {
            (*it)->Update(deltaTime);
            if ((*it)->IsDead())
            {
                delete* it;
                it = m_attackEffects.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // 反転（dmg原画は右向き想定）
        if (m_object)
        {
            const bool textureIsRightFacing = true;
            m_object->SetFlipX(textureIsRightFacing != m_facingRight);
        }

        m_animator.Update(deltaTime);
        Chara::Update(deltaTime);

        // アニメ終了で待機へ
        if (m_animator.IsFinished())
        {
            m_state = State::Idle;
            m_heavyDashTimer = 0.0f;

            m_lockFacing = false;

            m_object->SetTexture("asset/Texture/player_idle.png");
            m_object->SetSpriteSheet(6, 6);
            ApplyVisualSize(m_scaleIdle);
            m_animator.Play(m_idleAnim);
        }

        CommitPad();
        return;
    }

    // ===== 移動入力 =====
    SM::Vector2 moveDir = GetMoveInput();
    const bool isMoving = (moveDir.LengthSquared() > 0.01f);

    // 攻撃中かどうか
    const bool isAttacking = (m_state == State::AttackLight || m_state == State::AttackHeavy);

    // 攻撃中でない時だけ向きを更新（攻撃中に逆方向入力で反転しないため）
    if (!isAttacking && isMoving)
        UpdateFacingFromMove(moveDir);

    // =========================
    // 強攻撃：チャージ中
    // =========================
    if (m_state == State::AttackHeavyCharge)
    {
        // チャージ中はロックした向きを維持
        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_lockedFacingRight);

        // ボタンを離したら攻撃開始
        const bool aHeld = (buttons & XINPUT_GAMEPAD_A) != 0;
        const bool aUp = (!aHeld) && ((m_prevPadButtons & XINPUT_GAMEPAD_A) != 0);

        if (aUp || Input::GetKeyRelease(VK_RETURN))
        {
            m_state = State::AttackHeavy;
            m_attackSEPlayed = false;

            m_animator.Play(m_heavyStartAnim);

            StartHeavyDash(moveDir);

            CommitPad();
            return;
        }

        for (auto it = m_attackEffects.begin(); it != m_attackEffects.end(); )
        {
            (*it)->Update(deltaTime);

            if ((*it)->IsDead())
            {
                delete* it;
                it = m_attackEffects.erase(it);
            }
            else
            {
                ++it;
            }
        }

        m_animator.Update(deltaTime);
        Chara::Update(deltaTime);

        CommitPad();
        return;
    }

    // =========================
    // 攻撃中
    // =========================
    if (m_state == State::AttackLight || m_state == State::AttackHeavy)
    {
        if (m_state == State::AttackHeavy)
            UpdateHeavyDash(deltaTime);

        // 攻撃SE
        if (!m_attackSEPlayed)
        {
            int hitFrame = 0;

            if (m_state == State::AttackLight) hitFrame = 6;
            else if (m_state == State::AttackHeavy) hitFrame = 10;

            if (m_animator.GetCurrentFrame() >= hitFrame)
            {
                if (m_state == State::AttackLight)
                    Sound::GetInstance()->Play(SOUND_LABEL_SE_ATTACK_LIGHT);
                else
                    Sound::GetInstance()->Play(SOUND_LABEL_SE_ATTACK_HEAVY);

                m_attackSEPlayed = true;
            }
        }

        // アニメ終了で待機へ
        if (m_animator.IsFinished())
        {
            const bool wasHeavy = (m_state == State::AttackHeavy);

            m_state = State::Idle;
            m_heavyDashTimer = 0.0f;

            m_lockFacing = false;

            m_object->SetTexture("asset/Texture/player_idle.png");
            m_object->SetSpriteSheet(6, 6);
            ApplyVisualSize(m_scaleIdle);
            m_animator.Play(m_idleAnim);

            // ✅ 強攻撃終了後：1秒間は接触で被弾アニメを出さない
            if (wasHeavy)
            {
                StartNoHitAnim(1.0f);
                if (m_hitInvTimer < 1.0f) m_hitInvTimer = 1.0f;
            }
        }

        const bool textureIsRightFacing = true;
        const bool flipX = (textureIsRightFacing != (m_lockFacing ? m_lockedFacingRight : m_facingRight));
        m_object->SetFlipX(flipX);

        m_animator.Update(deltaTime);
        Chara::Update(deltaTime);

        CommitPad();
        return;
    }

    // =========================
    // 攻撃開始（強攻撃優先）
    // =========================
    if (attackHeavyInput)
    {
        if (isMoving) UpdateFacingFromMove(moveDir);

        m_lockFacing = true;
        m_lockedFacingRight = m_facingRight;

        m_state = State::AttackHeavyCharge;

        m_object->SetTexture("asset/Texture/player_attack_heavy.png");
        m_object->SetSpriteSheet(6, 5);
        ApplyVisualSize(m_scaleHeavy);

        m_animator.Play(m_heavyChargeAnim);

        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_lockedFacingRight);

        CommitPad();
        return;
    }

    if (attackLightInput)
    {
        if (isMoving) UpdateFacingFromMove(moveDir);

        m_lockFacing = true;
        m_lockedFacingRight = m_facingRight;

        m_state = State::AttackLight;

        m_attackSEPlayed = false;

        m_object->SetTexture("asset/Texture/player_attack_light.png");
        m_object->SetSpriteSheet(6, 3);
        ApplyVisualSize(m_scaleLight);
        m_animator.Play(m_attackLightAnim);

        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_facingRight);

        CommitPad();
        return;
    }

    // =========================
    // 通常移動
    // =========================
    Move(moveDir, deltaTime);

    if (isMoving && m_state != State::Walk)
    {
        m_state = State::Walk;
        m_object->SetTexture("asset/Texture/player_walk.png");
        m_object->SetSpriteSheet(3, 3);
        ApplyVisualSize(m_scaleWalk);
        m_animator.Play(m_walkAnim);
    }
    else if (!isMoving && m_state != State::Idle)
    {
        m_state = State::Idle;
        m_object->SetTexture("asset/Texture/player_idle.png");
        m_object->SetSpriteSheet(6, 6);
        ApplyVisualSize(m_scaleIdle);
        m_animator.Play(m_idleAnim);
    }

    const bool textureIsRightFacing = (m_state == State::Idle);
    m_object->SetFlipX(textureIsRightFacing != m_facingRight);

    m_animator.Update(deltaTime);
    Chara::Update(deltaTime);

    CommitPad();
}

int Player::GetAnimFrame() const
{
    return m_animator.GetCurrentFrame();
}

SM::Vector2 Player::GetMoveInput() const
{
    SM::Vector2 dir(0.0f, 0.0f);

    // ===== ゲームパッド（左スティック） =====
    XINPUT_STATE state{};
    DWORD res = XInputGetState(0, &state);

    if (res == ERROR_SUCCESS)
    {
        float lx = (float)state.Gamepad.sThumbLX;
        float ly = (float)state.Gamepad.sThumbLY;

        const float dead = (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

        if (fabsf(lx) < dead) lx = 0.0f;
        if (fabsf(ly) < dead) ly = 0.0f;

        const float maxv = 32767.0f;
        dir.x = lx / maxv;
        dir.y = ly / maxv;
    }

    // ===== キーボード（WASD） =====
    if (GetAsyncKeyState('W') & 0x8000) dir.y += 1.0f;
    if (GetAsyncKeyState('S') & 0x8000) dir.y -= 1.0f;
    if (GetAsyncKeyState('A') & 0x8000) dir.x -= 1.0f;
    if (GetAsyncKeyState('D') & 0x8000) dir.x += 1.0f;

    if (dir.LengthSquared() > 1.0f)
        dir.Normalize();

    return dir;
}

void Player::UpdateFacingFromMove(const SM::Vector2& moveDir)
{
    if (moveDir.x > 0.0f)      m_facingRight = true;
    else if (moveDir.x < 0.0f) m_facingRight = false;
}

void Player::StartHeavyDash(const SM::Vector2& moveDir)
{
    SM::Vector2 dir = moveDir;

    if (dir.LengthSquared() <= 0.01f)
        dir = m_facingRight ? SM::Vector2(1.0f, 0.0f) : SM::Vector2(-1.0f, 0.0f);
    else
        dir.Normalize();

    m_heavyDashDir = dir;
    m_heavyDashTimer = m_heavyDashDuration;
}

bool Player::UpdateHeavyDash(float deltaTime)
{
    if (!m_object) return false;
    if (m_heavyDashTimer <= 0.0f) return false;

    float dt = deltaTime;

    if (dt > 1.0f) dt *= 0.0001f;
    if (dt > 0.05f) dt = 0.05f;

    const float prevDash = m_heavyDashTimer;

    m_heavyDashTimer -= dt;
    if (m_heavyDashTimer < 0.0f) m_heavyDashTimer = 0.0f;

    // ✅ 強攻撃ダッシュが終わった瞬間から1秒は接触被弾アニメを禁止
    if (prevDash > 0.0f && m_heavyDashTimer == 0.0f)
    {
        StartNoHitAnim(1.0f);
        if (m_hitInvTimer < 1.0f) m_hitInvTimer = 1.0f;
    }

    auto p = m_object->GetPos();
    p.x += m_heavyDashDir.x * m_heavyDashSpeed * dt;
    p.y += m_heavyDashDir.y * m_heavyDashSpeed * dt;
    m_object->SetPos(p.x, p.y, p.z);

    return true;
}

void Player::Attack()
{
    // Mode / Skill 側で実装（ここでは未使用）
}

void Player::ApplyAbility(Skill* skill)
{
    if (!skill) return;
    skills.push_back(skill);
}

int Player::GetPower() const
{
    return power;
}

void Player::SetPower(int value)
{
    power = value;
}

bool Player::IsAttackInputTriggered() const
{
    return m_attackInputTriggered;
}

void Player::ApplyVisualSize(const SizeScale& s)
{
    if (!m_object) return;

    m_object->SetSize(m_baseW * s.sx, m_baseH * s.sy, 0.0f);

    // 当たり判定半径を固定値にする
    m_object->SetCollisionRadius(m_fixedRadius);
}
void Player::TakeDamage(int dmg)
{
    if (dmg <= 0) return;

    if (IsHeavyCharging() || IsHeavyDashing())
        return;

    if (m_invincibleTimer > 0.0f)
    {
        if (m_hitReactCD <= 0.0f)
        {
            m_state = State::Damaged;

            m_lockFacing = false;
            m_heavyDashTimer = 0.0f;

            if (m_object)
            {
                m_object->SetTexture("asset/Texture/player_damaged.png");
                m_object->SetSpriteSheet(5, 1);
                ApplyVisualSize(m_scaleDamaged);

                const bool textureIsRightFacing = true;
                m_object->SetFlipX(textureIsRightFacing != m_facingRight);
            }

            // Animatorが同一Play無視でも確実にリスタート
            m_animator.Play(m_idleAnim);
            m_animator.Play(m_damagedAnim);

            m_hitReactCD = m_hitReactCooldown;
        }
        return;
    }

    Chara::TakeDamage(dmg);
    if (hp <= 0) return;

    m_state = State::Damaged;

    m_lockFacing = false;
    m_heavyDashTimer = 0.0f;

    if (m_object)
    {
        m_object->SetTexture("asset/Texture/player_damaged.png");
        m_object->SetSpriteSheet(5, 1);
        ApplyVisualSize(m_scaleDamaged);

        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_facingRight);
    }

    m_animator.Play(m_idleAnim);
    m_animator.Play(m_damagedAnim);

    m_invincibleTimer = m_invincibleDuration;

    m_hitReactCD = m_hitReactCooldown;
}


void Player::PlayHitReaction()
{
    if (m_hitInvTimer > 0.0f) return;

    m_state = State::Damaged;
    m_lockFacing = false;
    m_heavyDashTimer = 0.0f;

    if (m_object)
    {
        m_object->SetTexture("asset/Texture/player_damaged.png");
        m_object->SetSpriteSheet(5, 1);
        ApplyVisualSize(m_scaleDamaged);

        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_facingRight);
    }

    m_animator.Play(m_idleAnim);
    m_animator.Play(m_damagedAnim);

    m_hitInvTimer = m_hitInvDuration;
}

