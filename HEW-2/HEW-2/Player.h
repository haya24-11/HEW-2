#pragma once

#include <vector>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <cstdint>

#include "Chara.h"
#include "Animator.h"
#include "Input.h"

class Skill;

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
    void ApplyAbility(Skill* skill);

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

private:
    // ===== アニメーション関連 =====
    Animator  m_animator;
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_attackLightAnim;
    Animation m_heavyChargeAnim;
    Animation m_heavyStartAnim;

    enum class State
    {
        Idle,
        Walk,
        AttackLight,
        AttackHeavyCharge,
        AttackHeavy,
    };
    State m_state = State::Idle;

    // 向き
    bool m_facingRight = true;

    // 成長要素
    int exp = 0;
    int level = 1;

    std::vector<Skill*> skills;

    // 基本サイズ
    float m_baseW = 130.0f;
    float m_baseH = 150.0f;

    // 衝突判定用の固定半径
    float m_fixedRadius = 15.0f;

    // 表示スケール補正
    SizeScale m_scaleIdle{ 1.0f, 1.0f };
    SizeScale m_scaleWalk{ 1.0f, 1.0f };
    SizeScale m_scaleLight{ 0.9f, 1.0f };
    SizeScale m_scaleHeavy{ 1.4f, 1.0f };

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

    // PAD入力（WORD 대신 안전하게)
    std::uint16_t m_prevPadButtons = 0;

    // 強攻撃時の方向固定
    bool m_lockFacing = false;
    bool m_lockedFacingRight = true;

    // ===== 強攻撃ヒット（前方判定） =====
    float m_heavyHitOffset = 80.0f;
    float m_heavyHitRadius = 55.0f;
    float m_heavyKnockBackPower = 900.0f; // ← 여기 깨져있던 부분 복구
};
