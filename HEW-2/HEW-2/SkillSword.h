#pragma once
#include"WeaponSkills.h"

class SkillSword:public WeaponSkills
{
private:

	float swordPower;
	DirectX::SimpleMath::Vector2 SwordArea;
public:
	
	SkillSword();

	float Getpower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;
};
