#include "AMode.h"

bool AMode::CanLearnSkill(int skillId) const
{
    return (skillId >= skillIdmin && skillId < skillIdMax);
}

bool AMode::CanUseSkill(const Skill& skill) const
{
    // ‚Æ‚è‚ ‚¦‚¸‘S•”Žg‚¦‚é
    return true;
}

int AMode::WeakAttack(int BaseAtk)
{
    return BaseAtk * weakAtk;
}

int AMode::StrongAttakc(int BaseAtk, float pushTime)
{
    return BaseAtk * strongAtkMin * pushTime;
}

float AMode::CalculateComboMultiplier(int comboCount) const
{
    return 1.0f + comboCount * 0.3f;
}