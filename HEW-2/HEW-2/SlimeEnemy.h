#pragma once
#include "Enemy.h"
class SlimeEnemy : public Enemy
{
public:
    SlimeEnemy();

protected:
    void SetupAnimation() override;
    void ApplyWalkVisual() override;
};

