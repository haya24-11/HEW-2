#include "Chara.h"

void Chara::Move(const Vector2& dir, float deltaTime)
{
    position.x += dir.x * deltaTime;
    position.y += dir.y * deltaTime;
}

int Chara::GetHp() const
{
    return hp;
}

bool Chara::IsAlive() const
{
    return status != Status::Dead;
}

void Chara::TakeDamage(int damage)
{
    hp -= damage;
    if (hp <= 0)
    {
        hp = 0;
        status = Status::Dead;
    }
}
