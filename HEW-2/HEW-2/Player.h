#pragma once
#include <vector>
#include "Chara.h"
#include "Animator.h"
#include "input.h"

class Skill;

/*
    Player
    ======
    �E�����v�f�����N���X
    �EExp
    �ELevel
    �ESkill ���X�g

    �� �퓬�����̒��g�͎����Ȃ�
*/

class Player : public Chara
{
public:
    Player();

    Object* GetObject() const { return m_object; }

    // ���͉��� �� Chara�����Ăяo��
    void Update(float deltaTime)override;

    int GetAnimFrame() const;

    // �U���̓���
    void Attack() override;

    // �X�L���擾���̋��ʏ���
    void ApplyAbility(Skill* skill);

    //�U���͂̃Q�b�^�[
    int GetAttck() const;

    void SetPower(int value);

private:
    // WASD���͂�����x�N�g���ɕϊ�
    DirectX::SimpleMath::Vector2 GetMoveInput() const;
private:
    // ===== Animation =====
    Animator m_animator;
    Animation m_idleAnim;
    Animation m_walkAnim;
    Animation m_attackLightAnim;
    Animation m_attackHeavyAnim;

    enum class State
    {
        Idle,
        Walk,
        AttackLight,      // ��U��
        AttackHeavy,    // ���U��
    };
    State m_state = State::Idle;

    enum class Facing
    {
        Right,
        Left
    };
    Facing m_facing = Facing::Right;
    bool m_facingRight = true; // �����͉E����

    int exp = 0;
    int level = 1;
    int m_animFrame = 0;     // ���݂̃t���[��
    float m_animTimer = 0;  // �o�ߎ���
    std::vector<Skill*> skills; // Skill�͏��L���Ȃ��iMode���Ǘ��j
};