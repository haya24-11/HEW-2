#include "SkillSword.h"

SkillSword::SkillSword() :WeaponSkills("Sword", 1), swordPower(1.5f), SwordArea(60.0f, 60.0f)
{
	
	
	
}

float SkillSword::Getpower() const
{
	return swordPower;
}

DirectX::SimpleMath::Vector2 SkillSword::GetArea() const
{
	return SwordArea;
}

