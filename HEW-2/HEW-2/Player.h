#pragma once

#include <vector>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <cstdint>

#include "Chara.h"
#include "Animator.h"
#include "Input.h"
#include "AttackSlashEffect.h"
#include "AttackDir.h"

class Skill;
class GamePlay; 

/*
    Player
    ======
    ・成長要素を持つプレイヤークラス
    ・経験値（Exp）
    ・レベル（Level）
    ・スキルリスト

    ※ 戦闘ロジックそのもの（当たり判定など）は持たない
*/

struct SizeScale
{
    float sx = 1.0f; // 横方向のスケール
    float sy = 1.0f; // 縦方向のスケール
};

class Player : public Chara
{
public:
    Player();
    virtual ~Player() = default;

    // 描画用 Object を取得（m_object は Chara 側にある前提）
    Object* GetObject() const { return m_object; }

    // 入力を解釈して Chara の処理を呼び出す
    void Update(float deltaTime) override;

    // 現在のアニメーションフレーム取得
    int GetAnimFrame() const;

    // =========================
    // 強攻撃用：GamePlay から参照
    // =========================

    // 強攻撃（ダッシュ）中かどうか
    bool IsHeavyDashing() const
    {
        return (m_state == State::AttackHeavy) && (m_heavyDashTimer > 0.0f);
    }

    // 強攻撃チャージ中かどうか
    bool IsHeavyCharging() const
    {
        return (m_state == State::AttackHeavyCharge);
    }

    // 強攻撃中のダッシュ速度（px/sec想定）
    DirectX::SimpleMath::Vector2 GetHeavyDashVelocity() const
    {
        return m_heavyDashDir * m_heavyDashSpeed;
    }

    // 強攻撃中の「固定向き」を返す（ロック中ならロック方向）
    bool GetFacingRightLocked() const
    {
        return m_lockFacing ? m_lockedFacingRight : m_facingRight;
    }

    // 強攻撃ヒット判定（前方円）のパラメータ
    float GetHeavyHitOffset() const { return m_heavyHitOffset; }
    float GetHeavyHitRadius() const { return m_heavyHitRadius; }

    // 敵を飛ばす強さ（ノックバック強度）
    float GetHeavyKnockBackPower() const { return m_heavyKnockBackPower; }

    // 攻撃処理の入口
    void Attack() override;

    // スキル取得時の共通処理
    void ApplyAbility(auto* skill);
   
    //取得したスキル一覧
    const std::vector<Skill*>& GetLearnedSkills() const;



    void ApplyAbility(Skill* skill);

    int GetPower() const;
    void SetPower(int value);
    
    int GetMaxHp() const { return m_maxHp; }

    float GetHeavyDamageMul() const { return m_heavyDamageMul; }
    void SetHeavyDamageMul(float v) { m_heavyDamageMul = v; }
    bool IsAttackInputTriggered() const;

    bool IsFacingRight() const { return m_facingRight; }

    bool ConsumeAttackEffectRequest();

    AttackDir GetAttackDir() const { return m_attackDir; }

    AttackDir GetHeavyDir() const { return m_attackDir; }
    // =========================
    // ✅ 被ダメージ
    // =========================
    void TakeDamage(int dmg);

    // ✅ 無敵判定（連続ヒット防止 + 強攻撃中無敵）
    void StartInvincible(float sec, bool blink)
    {
        if (sec <= 0.0f) return;
        if (m_invincibleTimer < sec) m_invincibleTimer = sec;
        m_invincibleBlink = blink;
    }

    bool IsInvincible() const { return m_invincibleTimer > 0.0f; }

    void PlayHitReaction();

    void StartNoHitAnim(float sec)
    {
        if (sec <= 0.0f) return;

        if (m_noHitAnimTimer < sec) m_noHitAnimTimer = sec;
        if (m_invincibleTimer < sec) m_invincibleTimer = sec;

        m_invincibleBlink = false; // ✅ 強攻撃後の無敵は点滅しない
    }

    bool IsNoHitAnim() const { return m_noHitAnimTimer > 0.0f; }

    void AddExp(int value);
    void LevelUp();

    int GetLevel() const { return m_level; }
    int GetCurrentExp() const { return m_currentExp; }
    int GetNextLevelExp() const;

    void SetGamePlay(GamePlay* gp) { m_gamePlay = gp; }
    GamePlay* GetGamePlay() const { return m_gamePlay; }

private:
    // WASD/Pad入力を移動方向ベクトルに変換
    DirectX::SimpleMath::Vector2 GetMoveInput() const;

    // 向き更新（入力方向から決める）
    void UpdateFacingFromMove(const DirectX::SimpleMath::Vector2& moveDir);

    // 強攻撃ダッシュ開始
    void StartHeavyDash(const DirectX::SimpleMath::Vector2& moveDir);

    // 強攻撃ダッシュ更新
    bool UpdateHeavyDash(float deltaTime);

    // 表示サイズを反映
    void ApplyVisualSize(const SizeScale& s);


    bool IsDamaged() const { return m_state == State::Damaged; }

private:
    // ===== アニメーション関連 =====
    Animator  m_animator;
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_attackLightAnim;
    Animation m_heavyChargeAnim;
    Animation m_heavyStartAnim;

    // ✅ 被ダメージ（横5枚 5x1）
    Animation m_damagedAnim = { 0, 5, 0.5f, false };

    enum class State
    {
        Idle,
        Walk,
        AttackLight,
        AttackHeavyCharge,
        AttackHeavy,
        AttackHeavyDash,    // 突進
        Damaged,
    };
    State m_state = State::Idle;

    AttackDir m_attackDir = AttackDir::Right;

    // 向き
    bool m_facingRight = true;

    // 成長要素
    int exp = 0;
    int level = 1;

    //マックスHP
    int m_maxHp = 50;  // ← hp と同じ初期値

    

    std::vector<Skill*> skills;

    // 基本サイズ
    float m_baseW = 130.0f;
    float m_baseH = 150.0f;

    // 衝突判定用の固定半径
    float m_fixedRadius = 60.0f;

    // 表示スケール補正
    SizeScale m_scaleIdle{ 1.0f, 1.0f };
    SizeScale m_scaleWalk{ 1.0f, 1.0f };
    SizeScale m_scaleLight{ 0.9f, 1.0f };
    SizeScale m_scaleHeavy{ 1.4f, 1.0f };
    SizeScale m_scaleDamaged{ 1.0f, 1.0f }; // ✅ 被ダメ用

    // ✅ 被ダメ後の短い無敵（連続ヒット防止）
    float m_invincibleTimer = 0.0f;
    float m_invincibleDuration = 5.0f;

    // ===== HeavyAttack ダッシュ =====
    float m_heavyDashSpeed = 700.0f;
    float m_heavyDashDuration = 0.18f;
    float m_heavyDashTimer = 0.0f;
    DirectX::SimpleMath::Vector2 m_heavyDashDir{ 1.0f, 0.0f };

    // ===== HeavyAttack チャージ =====
    float m_heavyChargeDuration = 1.0f;
    float m_heavyChargeTimer = 0.0f;

    int  m_heavyDashStartFrame = 14;
    bool m_heavyDashStarted = false;

    // PAD入力
    std::uint16_t m_prevPadButtons = 0;

    // 強攻撃時の方向固定
    bool m_lockFacing = false;
    bool m_lockedFacingRight = true;

    // ===== 強攻撃ヒット（前方判定） =====
    float m_heavyHitOffset = 80.0f;
    float m_heavyHitRadius = 55.0f;
    float m_heavyKnockBackPower = 900.0f;

    // 強攻撃調整
    float m_heavyDamageMul = 2.0f;

    // 攻撃エフェクト用
    std::vector<AttackSlashEffect*> m_attackEffects;

    bool m_attackInputTriggered = false;

    // ===== 攻撃SE制御 =====
    bool m_attackSEPlayed = false;

    // 弱攻撃ヒット遅延管理
    float m_attackLightTimer = 0.0f;
    bool  m_attackLightEffectFired = false;

    int m_prevAnimFrame = -1;   // 前フレーム記録用

    // 攻撃クールタイム
    float m_attackCooldown = 0.0f;
    const float m_attackCooldownTime = 0.35f; // 調整値

    bool m_attackEffectRequest = false;

    // 強攻撃制御
    bool  m_heavyHolding = false;
    bool  m_heavyReleased = false;

    float m_dashSpeed = 900.0f;
    float m_dashTime = 0.18f;
    float m_dashTimer = 0.0f;

    AttackDir m_heavyDir;

    // 強攻撃エフェクト1回制御 
    bool m_heavyEffectFired = false;

    Object* m_map = nullptr;

    float m_hitReactCD = 0.0f;
    float m_hitReactCooldown = 3.0f;

    float m_hitInvTimer = 0.0f;
    float m_hitInvDuration = 3.0f;

    float m_noHitAnimTimer = 0.0f;

    bool m_invincibleBlink = true;

    GamePlay* m_gamePlay = nullptr;

    // レベルシステム
    int m_level = 1;
    int m_currentExp = 0;

    bool m_justLeveledUp = false;


    float m_blinkTimer = 0.0f;        // 点滅用タイマー
    float m_blinkInterval = 3.0f;    // 点滅間隔（秒
    bool  m_blinkVisible = true;      // 今見えているか
};