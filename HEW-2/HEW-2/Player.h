#pragma once
#include <vector>
#include "Chara.h"
#include "Skill.h"

class Player : public Chara
{
public:
    void Attack() override;
    void ApplyAbility(const Skill& skill);

private:
    int exp;
    int level;
    std::vector<Skill> skills;
};