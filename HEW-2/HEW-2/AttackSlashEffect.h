#pragma once
#include <SimpleMath.h>
#include "Effect.h"

class AttackSlashEffect : public Effect
{
public:
    AttackSlashEffect(Scene* scene, Object* owner, bool facingRight);
    ~AttackSlashEffect();

    void Update(float deltaTime);
    bool IsDead() const { return m_dead; }
    void Uninit();

private:
    Object* m_object = nullptr;

    float m_timer = 0.0f;
    float m_lifeTime = 3.0f;   // 表示時間
    bool  m_visible = false;

    float m_delay = 0.08f;   // 表示遅延（秒）

    bool m_dead = false;
};

