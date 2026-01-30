#pragma once
#include <vector>
#include <string>
#include "Skill.h"

class Mode
{
public:
    virtual ~Mode() = default;

    virtual bool CanLearnSkill(int skillId) const = 0;
    virtual bool CanUseSkill(const Skill& skill) const = 0;
    virtual float CalculateComboMultiplier(int comboCount) const = 0;

};
