#include "Player.h"
#include "Skills.h"
#include "dinput.h"
#include <Windows.h>   // GetAsyncKeyState
#include <Xinput.h>    // XInput
#include <cmath>       // fabsf
#include "sound.h"
#include<iostream>
#pragma comment(lib, "Xinput.lib")

namespace SM = DirectX::SimpleMath;

Player::Player()
{
    hp = 100;
    power = 10;
    m_maxHp = 50;
    moveSpeed = 30.0f;

    // アニメーション（開始フレーム / フレーム数 / 1フレーム時間 / ループ）
    m_idleAnim = { 0, 30, 0.15f, true };
    m_walkAnim = { 0,  8, 0.50f, true };
    m_attackLightAnim = { 0, 15, 0.17f, false };
    m_heavyChargeAnim = { 0,  8, 0.20f, false };
    m_heavyStartAnim = { 8, 19, 0.20f, false };

    // 被ダメは m_damagedAnim(横5枚) を使用

    // レベルアップUIで選べるスキル一覧
    m_skillPool.push_back(new Skillpowerbuff(0));
    m_skillPool.push_back(new SkillKnockbackBuff);
    m_skillPool.push_back(new SkillSpeedBuff);
    m_skillPool.push_back(new SkillSpecialBuff);
}

Player::~Player()
{

}

void Player::Update(float deltaTime)
{

    // ===============================================
    // ✅ タイマー用deltaTimeをクランプ（無敵が一瞬で消えるのを防ぐ）
    //    ・描画/移動は元のdeltaTimeでOK（ここはタイマーだけ安定化）
    // ===============================================
    float tdt = deltaTime;

    // 既存の保険に合わせる（極端に大きいdeltaTimeを抑える）
    if (tdt > 1.0f) tdt *= 0.0001f;

    // HeavyDash側と同じ思想：上限0.05秒（20fps相当）に丸める
    if (tdt > 0.05f) tdt = 0.05f;

    // ===============================================
    // 強攻撃後の「被弾アニメ禁止」タイマー更新（GamePlay から参照）
    // ===============================================
    if (m_noHitAnimTimer > 0.0f)
    {
        m_noHitAnimTimer -= tdt;
        if (m_noHitAnimTimer < 0.0f) m_noHitAnimTimer = 0.0f;
    }

    // ===============================================
    // 無敵タイマー更新
    // ===============================================
    if (m_invincibleTimer > 0.0f)
    {
        m_invincibleTimer -= tdt;
        if (m_invincibleTimer < 0.0f) m_invincibleTimer = 0.0f;
    }

    // ===============================================
    // 無敵中の表示（点滅）
    // ===============================================
    if (m_object)
    {
        if (m_invincibleTimer > 0.0f)
        {
            if (m_invincibleBlink)
            {
                const float kBlink = 0.08f;               // 点滅の速さ
                const float t = fmodf(m_invincibleTimer, kBlink * 2.0f);
                const float a = (t < kBlink) ? 0.25f : 1.0f;
                m_object->SetColor(1.0f, 1.0f, 1.0f, a);  // ✅ 点滅あり
            }
            else
            {
                m_object->SetColor(1.0f, 1.0f, 1.0f, 1.0f); // ✅ 点滅なし（常に表示）
            }
        }
        else
        {
            m_object->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            m_invincibleBlink = true; // ✅ 次の無敵（被ダメ無敵）は点滅ありに戻す
        }
    }


    m_attackInputTriggered = false;

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

    // 強攻撃：A / 弱攻撃：B
    const bool attackHeavyPad = PadTrigger(XINPUT_GAMEPAD_A);
    const bool attackLightPad = PadTrigger(XINPUT_GAMEPAD_B);

    // ===== キーボード攻撃 =====
    const bool enterTrig = Input::GetKeyTrigger(VK_RETURN);
    const bool shiftHeld = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

    // 弱攻撃：Enter（ただし Shift を押していない時だけ）
    const bool attackLightKey = enterTrig && !shiftHeld;

    // 強攻撃：Shift + Enter
    const bool attackHeavyKey = enterTrig && shiftHeld;

    // ===== 攻撃入力（キーボード or パッド）=====
    // 強攻撃が入ったフレームは弱攻撃を必ず無効化して二重発動を防ぐ
    const bool attackHeavyInput = (attackHeavyKey || attackHeavyPad);
    const bool attackLightInput = (attackLightKey || attackLightPad) && !attackHeavyInput;

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

        // ボタンを離したら攻撃開始
        const bool aHeld = (buttons & XINPUT_GAMEPAD_A) != 0;
        const bool aUp = (!aHeld) && ((m_prevPadButtons & XINPUT_GAMEPAD_A) != 0);

        if (aUp || Input::GetKeyRelease(VK_RETURN))
        {
            m_state = State::AttackHeavy;
            m_heavyEffectFired = false;

            m_attackSEPlayed = false;

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

    // 押した瞬間に経験値が入る（デバッグ）
#ifdef _DEBUG

    if (GetAsyncKeyState(VK_F1) & 0x0001) AddExp(10);
    if (GetAsyncKeyState(VK_F2) & 0x0001) AddExp(100);
    if (GetAsyncKeyState(VK_F3) & 0x0001) AddExp(1000);

#endif

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
                StartNoHitAnim(6.0f);
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
    if (attackHeavyInput && m_attackCooldown <= 0.0f)
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

// =====================================================
// 経験値加算
// =====================================================
void Player::AddExp(int value)
{
    m_currentExp += value;

    // レベルアップ可能な限りループ
    while (m_currentExp >= GetNextLevelExp())
    {
        m_currentExp -= GetNextLevelExp();
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

    // ===== 今は確認用 =====
    printf("LEVEL UP! -> Lv %d\n", m_level);
    // ★ レベルアップUIを出すトリガー
    m_justLeveledUp = true;
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
    // 1フレームの移動が大きくなりすぎないように上限をかける
    if (dt > 0.05f) dt = 0.05f;

    const float prevDash = m_heavyDashTimer;

    m_heavyDashTimer -= dt;
    if (m_heavyDashTimer < 0.0f) m_heavyDashTimer = 0.0f;

    // ✅ 強攻撃ダッシュが終わった瞬間：被弾アニメ禁止＋完全無敵（HPも減らない）
    if (prevDash > 0.0f && m_heavyDashTimer == 0.0f)
    {
        StartNoHitAnim(1.5f);
        m_invincibleBlink = false;
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

std::vector<Skill*> Player::GetRandomSkillChoices(int count)
{
    std::vector<Skill*> result;

    // 1. 現在のスキルプールから「抽選可能なもの」だけを抽出
    std::vector<Skill*> validPool;
    for (auto* s : m_skillPool)
    {
        if (!s) continue;

        // 🔴 Lv3以上のスキルは候補に入れない
        if (s->GetLevel() >= 3) continue;

        // 🔴 特殊バフがすでに適用済みなら入れない
        if (dynamic_cast<SkillSpecialBuff*>(s) && SkillSpecialBuff::IsAlreadyApplied()) continue;

        validPool.push_back(s);
    }

    // 2. 抽選開始
    // 有効なスキルが要求数より少ない場合を考慮
    int actualCount = (std::min)(count, (int)validPool.size());

    for (int i = 0; i < actualCount; i++)
    {
        int idx = rand() % validPool.size();
        Skill* selected = validPool[idx];

        // 結果に追加
        result.push_back(selected);

        // 🔴 選んだスキルを一時プールから削除（同じ抽選回での重複を防止）
        validPool.erase(validPool.begin() + idx);
    }

    return result;
}

void Player::ApplyAbility(Skill* skill)
{
    // スキルを保持（適用）
    printf("[DEBUG] ApplyAbility Called!\n"); // ← これが出るか？

    if (!skill) {
        printf("[DEBUG] Skill is NULL!\n"); // ← もしこれが出たら UI から空が渡されている
        return;
    }

    skills.push_back(skill);

    printf("[DEBUG] Calling skill->Apply...\n"); // ← これが出るか？
    skill->Apply(this);
}

const std::vector<Skill*>& Player::GetLearnedSkills() const
{
    return skills;
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

bool Player::ConsumeAttackEffectRequest()
{
    if (!m_attackEffectRequest)
        return false;

    m_attackEffectRequest = false;
    return true;
}

void Player::TakeDamage(int dmg)
{
    if (dmg <= 0) return;

    // ✅ 強攻撃（チャージ含む）中はダッシュが終わっても被弾しない
    if (IsHeavyCharging() || (m_state == State::AttackHeavy))
    {
        std::cout << "[TakeDamage] BLOCKED(heavy-state) inv=" << m_invincibleTimer
            << " hp=" << hp << " dmg=" << dmg << "\n";
        return;
    }

    if (m_invincibleTimer > 0.0f)
    {
        std::cout << "[TakeDamage] BLOCKED(inv) inv=" << m_invincibleTimer
            << " hp=" << hp << " dmg=" << dmg << "\n";
        return;
    }

    std::cout << "[TakeDamage] APPLY inv=" << m_invincibleTimer
        << " hp=" << hp << " dmg=" << dmg << "\n";

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
    m_invincibleBlink = true;

    m_hitReactCD = m_hitReactCooldown;
}

void Player::PlayHitReaction()
{
    if (m_invincibleTimer > 0.0f) return;
    if (IsHeavyCharging() || IsHeavyDashing()) return;

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

}
int Player::GetNextLevelExp() const
{
    const float baseExp = 100.0f;
    const float growth = 1.35f; // ★調整ポイント

    return (int)(baseExp * powf((float)m_level, growth));
}
