#pragma once
#include "Skill.h"
#include "AMode.h"
class WeaponSkills : public Skill
{
protected:
    float basePower;     // Šî‘bˆĞ—Í
    float attackRange;   // UŒ‚”ÍˆÍi”¼Œa or ’·‚³j

public:
    WeaponSkills(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
    // UŒ‚ƒ_ƒ[ƒW‚ğŒvZ‚·‚éƒˆ‰¼‘zŠÖ”
    virtual int CalculateDamage(Player* player, AMode* mode) = 0;
};

