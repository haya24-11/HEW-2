#pragma once
#include "Skill.h"
#include "Player.h"

class Player;

class BuffSkill :
    public Skill
{
public:
    BuffSkill(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
    const char* GetIconPath() const override;
};

