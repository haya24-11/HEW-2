#include "Player.h"
#include "Skill.h"

#include <Windows.h>   // GetAsyncKeyState（キーボード入力取得）
#include <Xinput.h>    // XInput（ゲームパッド入力）
#include <cmath>       // fabsf
#include "sound.h"
#pragma comment(lib, "Xinput.lib")

namespace SM = DirectX::SimpleMath;

Player::Player()
{
    hp = 100;
    power = 10;
    //powerの2倍のダメージ   

    moveSpeed = 30.0f;


    // アニメーション（開始フレーム / フレーム数 / 1フレーム時間 / ループ）
    m_idleAnim = { 0, 30, 0.15f, true };
    m_walkAnim = { 0,  8, 0.50f, true };
    m_attackLightAnim = { 0, 15, 0.17f, false };
    m_heavyChargeAnim = { 0,  8, 0.20f, false };
    m_heavyStartAnim = { 8, 19, 0.20f, false };
}

void Player::Update(float deltaTime)
{
    m_attackEffectRequest = false;

    // 攻撃クールタイム更新
    if (m_attackCooldown > 0.0f)
        m_attackCooldown -= deltaTime;

    m_attackInputTriggered = false;

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

    //  強攻撃：A
    const bool attackHeavyPad = PadTrigger(XINPUT_GAMEPAD_A);
    //  弱攻撃：B
    const bool attackLightPad = PadTrigger(XINPUT_GAMEPAD_B);

    // ===== キーボード攻撃 =====
    const bool attackLightKey = Input::GetKeyTrigger(VK_RETURN);
    const bool attackHeavyKey = (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&Input::GetKeyTrigger(VK_RETURN);

    // 攻撃入力（キーボード or パッド）
    const bool attackLightInput = attackLightKey || attackLightPad;
    const bool attackHeavyInput = attackHeavyKey || attackHeavyPad;

    // ===== 移動入力 =====
    SM::Vector2 moveDir = GetMoveInput();
    const bool isMoving = (moveDir.LengthSquared() > 0.01f);

    // 攻撃中かどうか
    const bool isAttacking = (m_state == State::AttackLight || m_state == State::AttackHeavy);

    // 攻撃中でない時だけ向きを更新（攻撃中に逆方向入力で反転しないため）
    if (!isAttacking && isMoving)
        UpdateFacingFromMove(moveDir);

    // 早期 return が多いので、return直前に必ず前フレームボタンを更新する
    auto CommitPad = [&]()
        {
            m_prevPadButtons = buttons;
        };

    // =========================
    // 強攻撃：チャージ中
    // =========================
    if (m_state == State::AttackHeavyCharge)
    {
        // ======================
// ★チャージ中方向入力取得
// ======================
        SM::Vector2 inputDir = GetMoveInput();

        const float dead = 0.3f;

        bool up = inputDir.y > dead;
        bool down = inputDir.y < -dead;
        bool right = inputDir.x > dead;
        bool left = inputDir.x < -dead;

        // 8方向決定
        if (up && right)
            m_attackDir = AttackDir::UpRight;
        else if (up && left)
            m_attackDir = AttackDir::UpLeft;
        else if (down && right)
            m_attackDir = AttackDir::DownRight;
        else if (down && left)
            m_attackDir = AttackDir::DownLeft;
        else if (up)
            m_attackDir = AttackDir::Up;
        else if (down)
            m_attackDir = AttackDir::Down;
        else
            m_attackDir =
            m_lockedFacingRight ?
            AttackDir::Right :
            AttackDir::Left;

        // チャージ中は移動入力で向きを変えない（ロックした向きを維持）
        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_lockedFacingRight);

        // ボタンを離したら（Release）攻撃開始
        // ※ attackHeavyPad は「押した瞬間」判定なので、ホールド/リリース判定は別途行う
        const bool aHeld = (buttons & XINPUT_GAMEPAD_A) != 0;
        const bool aUp = (!aHeld) && ((m_prevPadButtons & XINPUT_GAMEPAD_A) != 0);

        if (aUp || Input::GetKeyRelease(VK_RETURN))
        {
            // 実際の攻撃モーションへ遷移
            m_state = State::AttackHeavy;
            m_heavyEffectFired = false;

            m_attackSEPlayed = false;

            // 攻撃アニメ（ループなし）
            m_animator.Play(m_heavyStartAnim);

            // ======================
            // 確定した攻撃方向で突進
            // ======================
            StartHeavyDash(
                AttackDirToVector(m_attackDir)
            );

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

        // チャージアニメを継続更新
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

        // ★ 弱攻撃エフェクト発生タイミング
        if (m_state == State::AttackLight)
        {
            int currentFrame = m_animator.GetCurrentFrame();
            // 攻撃エフェクトのフレーム調整
            int effectFrame = 4;

            // ★ フレームをまたいだ瞬間だけ発火
            if (!m_attackLightEffectFired &&
                m_prevAnimFrame < effectFrame &&
                currentFrame >= effectFrame)
            {
                m_attackEffectRequest = true;
                m_attackLightEffectFired = true;
            }

            // 最後に保存
            m_prevAnimFrame = currentFrame;
        }

        // 強攻撃中はダッシュ移動
        if (m_state == State::AttackHeavy)
        {
            UpdateHeavyDash(deltaTime);

            // ★強攻撃エフェクト1回生成
            if (!m_heavyEffectFired)
            {
                m_attackEffectRequest = true;
                m_heavyEffectFired = true;
            }
        }
        // ===== ★ 攻撃SE再生ここ ★ =====
        if (!m_attackSEPlayed)
        {
            int hitFrame = 0;

            if (m_state == State::AttackLight)
            {
                hitFrame = 6;   // 弱攻撃の当たりフレーム
            }
            else if (m_state == State::AttackHeavy)
            {
                hitFrame = 10;  // 強攻撃の当たりフレーム
            }

            if (m_animator.GetCurrentFrame() >= hitFrame)
            {
                if (m_state == State::AttackLight)
                    Sound::GetInstance()->Play(SOUND_LABEL_SE_ATTACK_LIGHT);
                else
                    Sound::GetInstance()->Play(SOUND_LABEL_SE_ATTACK_HEAVY);

                m_attackSEPlayed = true;
            }
        }

        // アニメ終了で待機に戻す
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

        // 攻撃/idle の原画は右向き想定 → 現在の向きと比較して反転
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
    if (attackHeavyInput && m_attackCooldown <= 0.0f)
    {
        // 攻撃開始瞬間の向きを決める（入力があればそちら、なければ現在の向き）
        if (isMoving) UpdateFacingFromMove(moveDir);

        // 攻撃中は向きを固定するためのロック
        m_lockFacing = true;
        m_lockedFacingRight = m_facingRight;

        // 強攻撃はまず「チャージ状態」に入る
        m_state = State::AttackHeavyCharge;

        m_object->SetTexture("asset/Texture/player_attack_heavy.png");
        m_object->SetSpriteSheet(6, 5);
        ApplyVisualSize(m_scaleHeavy);

        // チャージアニメ（ループ）
        m_animator.Play(m_heavyChargeAnim);

        // チャージ中も向き固定で反転
        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_lockedFacingRight);

        CommitPad();
        return;
    }

    if (attackLightInput && m_attackCooldown <= 0.0f)
    {

        // Player.cpp
        // 攻撃方向決定（上下優先）
        SM::Vector2 inputDir = GetMoveInput();

        // 上下入力を優先
        // ======================================
        // Player.cpp
        // 8方向攻撃判定
        // WASD / Pad 両対応
        // ======================================

        const float dead = 0.3f;

        bool up = inputDir.y > dead;
        bool down = inputDir.y < -dead;
        bool right = inputDir.x > dead;
        bool left = inputDir.x < -dead;

        if (up && right)
            m_attackDir = AttackDir::UpRight;
        else if (up && left)
            m_attackDir = AttackDir::UpLeft;
        else if (down && right)
            m_attackDir = AttackDir::DownRight;
        else if (down && left)
            m_attackDir = AttackDir::DownLeft;
        else if (up)
            m_attackDir = AttackDir::Up;
        else if (down)
            m_attackDir = AttackDir::Down;
        else
            m_attackDir =
            m_facingRight ?
            AttackDir::Right :
            AttackDir::Left;

        m_attackCooldown = m_attackCooldownTime;

        // 攻撃開始瞬間の向きを決める
        if (isMoving) UpdateFacingFromMove(moveDir);

        m_attackLightTimer = 0.0f;
        m_attackLightEffectFired = false;
        m_prevAnimFrame = -1;

        // 攻撃中は向きを固定
        m_lockFacing = true;
        m_lockedFacingRight = m_facingRight;

        m_state = State::AttackLight;

        m_attackSEPlayed = false;

        m_object->SetTexture("asset/Texture/player_attack_light.png");
        m_object->SetSpriteSheet(6, 3);
        ApplyVisualSize(m_scaleLight);
        m_animator.Play(m_attackLightAnim);

        // 攻撃開始フレームでも反転を適用
        const bool textureIsRightFacing = true;
        m_object->SetFlipX(textureIsRightFacing != m_facingRight);

        CommitPad();
        return;
    }

    // =========================
    // 通常移動
    // =========================
    Move(moveDir, deltaTime);

    // 移動/待機でテクスチャ＆シートを切り替え
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

    // 毎フレーム反転を適用（向きが変わると見た目も反転）
    // ※ walk シートの原画向きが idle と違う場合は、この判定を調整する
    const bool textureIsRightFacing = (m_state == State::Idle); // 必要に応じて変更
    m_object->SetFlipX(textureIsRightFacing != m_facingRight);

    m_animator.Update(deltaTime);
    Chara::Update(deltaTime);

    CommitPad();
}

int Player::GetAnimFrame() const
{
    // 現在のアニメフレーム番号を返す（Draw(frame) 用）
    return m_animator.GetCurrentFrame();
}

// =====================================================
// 経験値加算
// =====================================================
void Player::AddExp(int value)
{
    m_currentExp += value;

    // レベルアップ可能な限りループ
    while (m_currentExp >= m_nextLevelExp)
    {
        m_currentExp -= m_nextLevelExp;
        LevelUp();
        OutputDebugStringA("Level Up!\n");
    }
}

// =====================================================
// レベルアップ
// =====================================================
void Player::LevelUp()
{
    m_level++;

    // 次レベル必要EXP増加（基本式）
    m_nextLevelExp = static_cast<int>(m_nextLevelExp * 1.5f);

    // ===== 今は確認用 =====
    printf("LEVEL UP! -> Lv %d\n", m_level);
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

        // デッドゾーン（小さな入力を無視）
        const float dead = (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

        if (fabsf(lx) < dead) lx = 0.0f;
        if (fabsf(ly) < dead) ly = 0.0f;

        // -32768～32767 を -1～1 に正規化
        const float maxv = 32767.0f;
        dir.x = lx / maxv;
        dir.y = ly / maxv;
    }

    // ===== キーボード（WASD） =====
    if (GetAsyncKeyState('W') & 0x8000) dir.y += 1.0f;
    if (GetAsyncKeyState('S') & 0x8000) dir.y -= 1.0f;
    if (GetAsyncKeyState('A') & 0x8000) dir.x -= 1.0f;
    if (GetAsyncKeyState('D') & 0x8000) dir.x += 1.0f;

    // 斜め移動で速度が速くならないように正規化
    if (dir.LengthSquared() > 1.0f)
        dir.Normalize();

    return dir;
}

void Player::UpdateFacingFromMove(const SM::Vector2& moveDir)
{
    // 移動入力から左右向きを更新（x成分だけを見る）
    if (moveDir.x > 0.0f)      m_facingRight = true;
    else if (moveDir.x < 0.0f) m_facingRight = false;
}

void Player::StartHeavyDash(const SM::Vector2& moveDir)
{
    SM::Vector2 dir = moveDir;

    // 入力がない場合は現在の向きにダッシュ
    if (dir.LengthSquared() <= 0.01f)
        dir = m_facingRight ? SM::Vector2(1.0f, 0.0f) : SM::Vector2(-1.0f, 0.0f);
    else
        dir.Normalize();

    // ダッシュ方向＆残り時間を設定
    m_heavyDashDir = dir;
    m_heavyDashTimer = m_heavyDashDuration;

}

bool Player::UpdateHeavyDash(float deltaTime)
{
    if (!m_object) return false;
    if (m_heavyDashTimer <= 0.0f) return false;

    float dt = deltaTime;

    // 異常な deltaTime 対策（超大きい値が来た場合の保険）
    if (dt > 1.0f) dt *= 0.0001f;
    // 1フレームの移動が大きくなりすぎないように上限をかける
    if (dt > 0.05f) dt = 0.05f;

    // タイマー減算
    m_heavyDashTimer -= dt;
    if (m_heavyDashTimer < 0.0f) m_heavyDashTimer = 0.0f;

    // 方向×速度×時間で移動
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
    // スキルを保持（適用）
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

    // 見た目サイズを更新
    m_object->SetSize(m_baseW * s.sx, m_baseH * s.sy, 0.0f);

    // 当たり判定半径を固定値にする（スプライトサイズ変更でも判定が変わらない）
    m_object->SetCollisionRadius(m_fixedRadius);
}

bool Player::ConsumeAttackEffectRequest()
{
    if (!m_attackEffectRequest)
        return false;

    m_attackEffectRequest = false;
    return true;
}

