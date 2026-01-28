/*
#include "Player.h"
#include "Skill.h"

#include <Windows.h>   // GetAsyncKeyState, VK_*
#include <Xinput.h>
#include <cmath>       // fabsf
#pragma comment(lib, "Xinput.lib")

using DirectX::SimpleMath::Vector2;

Player::Player()
{
    hp = 100;
    power = 10;

    moveSpeed = 15.0f; // Player 固有の速さ

    // ===== Animation定義 =====
    m_idleAnim = { 0, 30, 0.15f, true };  // idle
    m_walkAnim = { 0,  8, 0.50f, true };  // walk
    m_attackLightAnim = { 0, 15, 0.17f, false };  // light
    m_attackHeavyAnim = { 0, 27, 0.20f, false };  // heavy
}

void Player::Update(float deltaTime)
{
    // 先に移動入力（向き更新・強攻撃ダッシュ方向にも使う）
    Vector2 moveDir = GetMoveInput();
    const bool isMoving = (moveDir.LengthSquared() > 0.01f);

    if (isMoving)
        UpdateFacingFromMove(moveDir);

    // 入力（攻撃）
    const bool attackLightInput = Input::GetKeyTrigger(VK_RETURN);
    const bool attackHeavyInput =
        (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
        Input::GetKeyTrigger(VK_RETURN);

    // =========================
    // Attack中の処理
    // =========================
    if (m_state == State::AttackLight || m_state == State::AttackHeavy)
    {
        // 強攻撃中は、最初だけ突進
        if (m_state == State::AttackHeavy)
        {
            UpdateHeavyDash(deltaTime);
        }

        // アニメが終わったら Idle に戻す
        if (m_animator.IsFinished())
        {
            m_state = State::Idle;
            m_heavyDashTimer = 0.0f;

            m_object->SetTexture("asset/Texture/player_idle.png");
            m_object->SetSpriteSheet(6, 6);
            ApplyVisualSize(m_scaleIdle);
            m_animator.Play(m_idleAnim);
        }

        // 向き反映（idle/attackは右向き原画）
        {
            const bool textureIsRightFacing = true;
            const bool flipX = (textureIsRightFacing != m_facingRight);
            m_object->SetFlipX(flipX);
        }

        m_animator.Update(deltaTime);
        Chara::Update(deltaTime);
        return;
    }

    // =========================
    // Attack開始判定（強攻撃優先）
    // =========================
    if (attackHeavyInput)
    {
        m_state = State::AttackHeavy;

        m_object->SetTexture("asset/Texture/player_attack_heavy.png");
        m_object->SetSpriteSheet(6, 5);
        ApplyVisualSize(m_scaleHeavy);
        m_animator.Play(m_attackHeavyAnim);

        // ✅ 強攻撃ダッシュ開始
        StartHeavyDash(moveDir);

        return;
    }

    if (attackLightInput)
    {
        m_state = State::AttackLight;

        m_object->SetTexture("asset/Texture/player_attack_light.png");
        m_object->SetSpriteSheet(6, 3);
        ApplyVisualSize(m_scaleLight);
        m_animator.Play(m_attackLightAnim);

        return;
    }

    // =========================
    // 通常移動
    // =========================
    Move(moveDir, deltaTime);

    // 状態遷移（Walk / Idle）
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

    // 向き反映
    // Idle/Attack系: 右向き原画
    // Walk: 左向き原画（プロジェクトの仕様に合わせて）
    {
        const bool textureIsRightFacing = (m_state == State::Idle);
        const bool flipX = (textureIsRightFacing != m_facingRight);
        m_object->SetFlipX(flipX);
    }

    m_animator.Update(deltaTime);
    Chara::Update(deltaTime);
}

int Player::GetAnimFrame() const
{
    return m_animator.GetCurrentFrame();
}

Vector2 Player::GetMoveInput() const
{
    Vector2 dir(0.0f, 0.0f);

    // =========================
    // (A) ゲームパッド（XInput）
    // =========================
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

    // =========================
    // (B) キーボード（WASD）
    // =========================
    if (GetAsyncKeyState('W') & 0x8000) dir.y += 1.0f;
    if (GetAsyncKeyState('S') & 0x8000) dir.y -= 1.0f;
    if (GetAsyncKeyState('A') & 0x8000) dir.x -= 1.0f;
    if (GetAsyncKeyState('D') & 0x8000) dir.x += 1.0f;

    // =========================
    // (C) 正規化（斜め速度一定）
    // =========================
    if (dir.LengthSquared() > 1.0f)
        dir.Normalize();

    return dir;
}

void Player::UpdateFacingFromMove(const Vector2& moveDir)
{
    if (moveDir.x > 0.0f)      m_facingRight = true;
    else if (moveDir.x < 0.0f) m_facingRight = false;
}

void Player::StartHeavyDash(const Vector2& moveDir)
{
    // ダッシュ方向：入力があれば入力方向、無ければ向いてる方向
    Vector2 dir = moveDir;

    if (dir.LengthSquared() <= 0.01f)
    {
        dir = m_facingRight ? Vector2(1.0f, 0.0f) : Vector2(-1.0f, 0.0f);
    }
    else
    {
        dir.Normalize();
    }

    m_heavyDashDir = dir;
    m_heavyDashTimer = m_heavyDashDuration;
}

bool Player::UpdateHeavyDash(float deltaTime)
{
    if (m_heavyDashTimer <= 0.0f)
        return false;

    m_heavyDashTimer -= deltaTime;
    if (m_heavyDashTimer < 0.0f)
        m_heavyDashTimer = 0.0f;

    // 前方へ移動
    auto p = m_object->GetPos();
    p.x += m_heavyDashDir.x * m_heavyDashSpeed * deltaTime;
    p.y += m_heavyDashDir.y * m_heavyDashSpeed * deltaTime;
    m_object->SetPos(p.x, p.y, p.z);

    return true;
}

void Player::Attack()
{
    // 実際の攻撃内容は Mode / Skill 側が決める
}

void Player::ApplyAbility(Skill* skill)
{
    if (!skill) return;
    skills.push_back(skill);
}

void Player::ApplyVisualSize(const SizeScale& s)
{
    if (!m_object) return;

    m_object->SetSize(m_baseW * s.sx, m_baseH * s.sy, 0.0f);
    m_object->SetCollisionRadius(m_fixedRadius);
}
*/

// Player.cpp
#include "Player.h"
#include "Skill.h"

#include <Windows.h>   // GetAsyncKeyState
#include <Xinput.h>
#include <cmath>       // fabsf
#pragma comment(lib, "Xinput.lib")

namespace SM = DirectX::SimpleMath;

Player::Player()
{
    hp = 100;
    power = 10;

    moveSpeed = 15.0f;

    // Animation
    m_idleAnim = { 0, 30, 0.15f, true };
    m_walkAnim = { 0,  8, 0.50f, true };
    m_attackLightAnim = { 0, 15, 0.17f, false };
    m_attackHeavyAnim = { 0, 27, 0.20f, false };
}

void Player::Update(float deltaTime)
{
    SM::Vector2 moveDir = GetMoveInput();
    const bool isMoving = (moveDir.LengthSquared() > 0.01f);

    if (isMoving)
        UpdateFacingFromMove(moveDir);

    // Attack input
    const bool attackLightInput = Input::GetKeyTrigger(VK_RETURN);
    const bool attackHeavyInput =
        (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
        Input::GetKeyTrigger(VK_RETURN);

    // =========================
    // Attack中
    // =========================
    if (m_state == State::AttackLight || m_state == State::AttackHeavy)
    {
        if (m_state == State::AttackHeavy)
        {
            UpdateHeavyDash(deltaTime);
        }

        if (m_animator.IsFinished())
        {
            m_state = State::Idle;
            m_heavyDashTimer = 0.0f;

            m_object->SetTexture("asset/Texture/player_idle.png");
            m_object->SetSpriteSheet(6, 6);
            ApplyVisualSize(m_scaleIdle);
            m_animator.Play(m_idleAnim);
        }

        const bool textureIsRightFacing = true;
        const bool flipX = (textureIsRightFacing != m_facingRight);
        m_object->SetFlipX(flipX);

        m_animator.Update(deltaTime);
        Chara::Update(deltaTime);
        return;
    }

    // =========================
    // Attack開始（Heavy優先）
    // =========================
    if (attackHeavyInput)
    {
        m_state = State::AttackHeavy;

        m_object->SetTexture("asset/Texture/player_attack_heavy.png");
        m_object->SetSpriteSheet(6, 5);

        ApplyVisualSize(m_scaleHeavy);

        m_animator.Play(m_attackHeavyAnim);

        StartHeavyDash(moveDir);
        return;
    }

    if (attackLightInput)
    {
        m_state = State::AttackLight;

        m_object->SetTexture("asset/Texture/player_attack_light.png");
        m_object->SetSpriteSheet(6, 3);
        ApplyVisualSize(m_scaleLight);
        m_animator.Play(m_attackLightAnim);
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
    const bool flipX = (textureIsRightFacing != m_facingRight);
    m_object->SetFlipX(flipX);

    m_animator.Update(deltaTime);
    Chara::Update(deltaTime);
}

int Player::GetAnimFrame() const
{
    return m_animator.GetCurrentFrame();
}

SM::Vector2 Player::GetMoveInput() const
{
    SM::Vector2 dir(0.0f, 0.0f);

    // Gamepad
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

    // Keyboard
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

    m_heavyDashTimer -= dt;
    if (m_heavyDashTimer < 0.0f) m_heavyDashTimer = 0.0f;

    auto p = m_object->GetPos();
    p.x += m_heavyDashDir.x * m_heavyDashSpeed * dt;
    p.y += m_heavyDashDir.y * m_heavyDashSpeed * dt;
    m_object->SetPos(p.x, p.y, p.z);

    return true;
}


void Player::Attack()
{
    // Mode / Skill 側で実装
}

void Player::ApplyAbility(Skill* skill) 
{
    if (!skill) return;
    skills.push_back(skill);
}

void Player::ApplyVisualSize(const SizeScale& s)
{
    if (!m_object) return;

    m_object->SetSize(m_baseW * s.sx, m_baseH * s.sy, 0.0f);
    m_object->SetCollisionRadius(m_fixedRadius);
}
