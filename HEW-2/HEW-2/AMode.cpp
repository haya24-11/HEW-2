#include "AMode.h"

bool AMode::CanLearnSkill(int skillId) const
{
	return (skillId >= 0 && skillId < 10);
}

int AMode::WeakAttack(int BaseAtk)
{
	return BaseAtk* weakAtk;
}

int AMode::StrongAttakc(int BaseAtk,float pushTime)
{
	return BaseAtk * strongAtkMin * pushTime;
}

float AMode::CalculateComboMultiplier(int comboCount) const
{
	return 1.0f + comboCount * 0.3f;

}
