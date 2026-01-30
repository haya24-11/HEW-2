#pragma once
#include"WeaponSkills.h"

class SkillAxe:public WeaponSkills
{
public:
	SkillAxe();
	int CalculateDamage(Player* player, AMode* mode) override;
};
