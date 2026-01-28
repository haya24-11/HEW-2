#pragma once
#include "Skill.h"
class WeaponSkills : public Skill
{
public:
    WeaponSkills(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
};

