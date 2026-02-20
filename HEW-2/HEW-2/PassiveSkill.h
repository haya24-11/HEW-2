#pragma once
#include "Skill.h"
class PassiveSkill :
    public Skill
{
public:
    PassiveSkill(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
};

