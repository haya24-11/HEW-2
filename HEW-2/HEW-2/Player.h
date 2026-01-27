#pragma once
#include <vector>
#include "Chara.h"
#include "Animator.h"

class Skill;

/*
    Player
    ======
    ・成長要素を持つクラス
    ・Exp
    ・Level
    ・Skill リスト

    ※ 戦闘処理の中身は持たない
*/

class Player : public Chara
{
public:
    Player();

    Object* GetObject() const { return m_object; }

    // 入力解釈 ⇒ Chara処理呼び出し
    void Update(float deltaTime)override;

    int GetAnimFrame() const;

    // 攻撃の入口
    void Attack() override;

    // スキル取得時の共通処理
    void ApplyAbility(Skill* skill);

private:
    // WASD入力を方向ベクトルに変換
    DirectX::SimpleMath::Vector2 GetMoveInput() const;
private:
    // ===== Animation =====
    Animator m_animator;
    Animation m_idleAnim;
    Animation m_walkAnim;
    
    enum class State
    {
        Idle,
        Walk
    };
    State m_state = State::Idle;

    enum class Facing
    {
        Right,
        Left
    };
    Facing m_facing = Facing::Right;
    bool m_facingRight = true; // 初期は右向き

    int exp = 0;
    int level = 1;
    int m_animFrame = 0;     // 現在のフレーム
    float m_animTimer = 0;  // 経過時間
    
    std::vector<Skill*> skills; // Skillは所有しない（Mode側管理）
};  