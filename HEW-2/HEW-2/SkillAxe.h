#pragma once
#include"WeaponSkills.h"
#include "AMode.h"

class SkillAxe:public WeaponSkills
{
public:
	SkillAxe();
	int CalculateDamage(Player* player, AMode* mode) override;
};
