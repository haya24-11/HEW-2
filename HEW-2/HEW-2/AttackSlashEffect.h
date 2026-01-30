#pragma once
#include "Object.h"
#include <SimpleMath.h>

class AttackSlashEffect
{
public:
    AttackSlashEffect(Object* owner, bool facingRight);
    ~AttackSlashEffect();

    void Update(float deltaTime);
    bool IsDead() const { return m_dead; }

private:
    Object* m_object = nullptr;

    float m_timer = 0.0f;
    float m_lifeTime = 0.2f;   // •\Ž¦ŽžŠÔ

    bool m_dead = false;
};

