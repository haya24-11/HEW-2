#pragma once
#include "Chara.h"

class Enemy : public Chara
{
public:
    void Attack() override;
    void KnockBack(const Vector2& dir);

protected:
    bool isBoss;
};