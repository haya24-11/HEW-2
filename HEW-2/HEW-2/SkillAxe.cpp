#include "SkillAxe.h"
#include "AMode.h"

SkillAxe::SkillAxe():WeaponSkills("Aex", 2)
{
	basePower = 1.5f;
	attackRange = 2.0f; 
}

int SkillAxe::CalculateDamage(Player* player, AMode* mode)
{
	int base = player->GetPower() * basePower;
	return mode->WeakAttack(base);  // ÉÇÅ[Éhï‚ê≥

}

