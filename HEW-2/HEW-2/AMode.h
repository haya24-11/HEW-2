#pragma once
#include "Mode.h"

class AMode : public Mode
{
    // Mode‚²‚Æ‚Ì“Áêƒ‹[ƒ‹
private:
   int weakAtk = 2;
   int strongAtkMin = 3;
   int skillIdmin = 0;
   int skillIdMax = 10;

public:
    bool CanLearnSkill(int skillId) const override;
    bool CanUseSkill(const Skill& skill) const override;
    float CalculateComboMultiplier(int comboCount) const override;

    int WeakAttack(int BaseAtk);
    int StrongAttakc(int BaseAtk, float pushTime);
};