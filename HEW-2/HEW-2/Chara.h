#pragma once
#include "GameObject.h"
#include "GameTypes.h"

class Chara : public GameObject
{
public:
    virtual ~Chara() = default;

    virtual void Move(const Vector2& dir, float deltaTime);
    virtual void Attack() = 0;

    int GetHp() const;
    bool IsAlive() const;

    void TakeDamage(int damage);

protected:
    int hp = 1;
    int power = 1;
    Status status = Status::Normal;
};;