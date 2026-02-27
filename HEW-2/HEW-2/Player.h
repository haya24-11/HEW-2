/*
#pragma once
#include <vector>
#include "Chara.h"
#include "Animator.h"
#include "input.h"

class Skill;


    //Player
    //======
    //・成長要素を持つプレイヤークラス
    //・経験値（Exp）
    //・レベル（Level）
    //・スキルリスト

    //※ 戦闘ロジックそのもの（当たり判定など）は持たない

struct SizeScale
{
    float sx = 1.0f; // 横方向のスケール
    float sy = 1.0f; // 縦方向のスケール
};

class Player : public Chara
{
public:
    Player();

    // 描画用 Object を取得
    Object* GetObject() const { return m_object; }

    // 入力を解釈して Chara の処理を呼び出す
    void Update(float deltaTime) override;

    // 現在のアニメーションフレーム取得
    int GetAnimFrame() const;

    // 攻撃処理の入口
    void Attack() override;

    // スキル取得時の共通処理
    void ApplyAbility(Skill* skill);

    int GetPower() const;

    void Setpower(int value);
private:
    // WASD入力を移動方向ベクトルに変換
    DirectX::SimpleMath::Vector2 GetMoveInput() const;

private:
    // ===== アニメーション関連 =====
    Animator m_animator;
    Animation m_idleAnim;          // 待機アニメーション
    Animation m_walkAnim;          // 移動アニメーション
    Animation m_attackLightAnim;   // 弱攻撃アニメーション
    Animation m_attackHeavyAnim;   // 強攻撃アニメーション

    // プレイヤーの状態
    enum class State
    {
        Idle,           // 待機
        Walk,           // 移動
        AttackLight,    // 弱攻撃
        AttackHeavy,    // 強攻撃
    };
    State m_state = State::Idle;

    // 向き
    enum class Facing
    {
        Right,  // 右向き
        Left    // 左向き
    };
    Facing m_facing = Facing::Right;
    bool m_facingRight = true; // 初期状態は右向き

    // 成長要素
    int exp = 0;        // 経験値
    int level = 1;      // レベル

    // アニメーション制御
    int m_animFrame = 0;     // 現在のフレーム
    float m_animTimer = 0;  // 経過時間

    // 所持スキル（Skillの管理自体は Mode 側）
    std::vector<Skill*> skills;

    // 基本サイズ
    float m_baseW = 130.0f;
    float m_baseH = 150.0f;

    // 衝突判定用の固定半径
    float m_fixedRadius = 15.0f;

    // 状態ごとの表示スケール補正
    SizeScale m_scaleIdle{ 1.0f, 1.0f };   // 待機時
    SizeScale m_scaleWalk{ 1.0f, 1.0f };   // 移動時
    SizeScale m_scaleLight{ 0.9f, 1.0f };  // 弱攻撃時の補正
    SizeScale m_scaleHeavy{ 1.4f, 1.0f };  // 強攻撃時の補正

    // 表示サイズを反映
    void ApplyVisualSize(const SizeScale& s);
};*/
#pragma once

#include <vector>
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "Chara.h"
#include "Animator.h"
#include "Input.h"   // ※プロジェクト側のファイル名に合わせて (Input.h / input.h)

#include <Xinput.h>

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
    virtual ~Player();

    // 描画用 Object を取得（m_object は Chara 側にある前提）
    Object* GetObject() const { return m_object; }

    // 入力を解釈して Chara の処理を呼び出す
    void Update(float deltaTime) override;

    // 現在のアニメーションフレーム取得
    int GetAnimFrame() const;

    // 攻撃処理の入口
    void Attack() override;

    // スキル取得時の共通処理
    void ApplyAbility(Skill* skill);
   
    //取得したスキル一覧
    const std::vector<Skill*>& GetLearnedSkills() const;



    int GetPower() const;

    void SetPower(int value);

private:
    // WASD/Pad入力を移動方向ベクトルに変換
    DirectX::SimpleMath::Vector2 GetMoveInput() const;

    // 向き更新（入力方向から決める）
    void UpdateFacingFromMove(const DirectX::SimpleMath::Vector2& moveDir);

    // =========================
    // HeavyAttack ダッシュ関連
    // =========================

    /*
        StartHeavyDash
        --------------
        ・強攻撃開始時に呼ぶ
        ・現在の向き/入力に応じてダッシュ方向を確定する
    */
    void StartHeavyDash(const DirectX::SimpleMath::Vector2& moveDir);

    /*
        UpdateHeavyDash
        --------------
        ・強攻撃中に呼ぶ
        ・タイマーが残っている間だけ前方へ移動させる
        ・戻り値 true : ダッシュ中（通常処理を止める）
        ・戻り値 false: ダッシュ終了
    */
    bool UpdateHeavyDash(float deltaTime);

    // 表示サイズを反映
    void ApplyVisualSize(const SizeScale& s);

private:
    // ===== アニメーション関連 =====
    Animator  m_animator;
    Animation m_idleAnim;          // 待機アニメーション
    Animation m_walkAnim;          // 移動アニメーション
    Animation m_attackLightAnim;   // 弱攻撃アニメーション
    Animation m_heavyChargeAnim;   // 強攻撃チャージアニメーション
    Animation m_heavyStartAnim;    // 強攻撃アニメーション
    // プレイヤーの状態
    enum class State
    {
        Idle,               // 待機
        Walk,               // 移動
        AttackLight,        // 弱攻撃
        AttackHeavyCharge,  // 強攻撃チャージ
        AttackHeavy,        // 強攻撃
    };
    State m_state = State::Idle;

    // 向き
    bool m_facingRight = true; // true:右向き / false:左向き

    // 成長要素
    int exp = 0;        // 経験値
    int level = 1;      // レベル

    // 所持スキル 所有権を持つ
    std::vector<Skill*> skills;

    // 基本サイズ
    float m_baseW = 130.0f;
    float m_baseH = 150.0f;

    // 衝突判定用の固定半径
    float m_fixedRadius = 15.0f;

    // 状態ごとの表示スケール補正
    SizeScale m_scaleIdle{ 1.0f, 1.0f };   // 待機時
    SizeScale m_scaleWalk{ 1.0f, 1.0f };   // 移動時
    SizeScale m_scaleLight{ 0.9f, 1.0f };  // 弱攻撃時の補正
    SizeScale m_scaleHeavy{ 1.4f, 1.0f };  // 強攻撃時の補正

    // =========================
    // HeavyAttack ダッシュパラメータ
    // =========================
    float m_heavyDashSpeed = 700.0f;  // ダッシュ速度（px/sec想定）
    float m_heavyDashDuration = 0.18f;   // ダッシュ継続時間（秒）
    float m_heavyDashTimer = 0.0f;    // 残り時間
    DirectX::SimpleMath::Vector2 m_heavyDashDir{ 1.0f, 0.0f }; // ダッシュ方向

    // =========================
// HeavyAttack チャージ（溜め）
// =========================
    float m_heavyChargeDuration = 1.0f;     
    float m_heavyChargeTimer = 0.0f;    


    int  m_heavyDashStartFrame = 14; 
    bool m_heavyDashStarted = false; 

    //PAD入力
    WORD m_prevPadButtons = 0;

    //強攻撃時の方向固定
    bool m_lockFacing = false;
    bool m_lockedFacingRight = true;

};
