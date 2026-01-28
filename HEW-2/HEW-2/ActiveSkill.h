#pragma once
#include "Skill.h"
class ActiveSkill :
    public Skill
{
public:
    ActiveSkill(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
};