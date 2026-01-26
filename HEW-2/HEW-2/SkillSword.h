#pragma once
#include"Skill.h"

class SkillSword:public Skill
{
private:

	float swordPower;
	DirectX::SimpleMath::Vector2 SwordArea;
public:
	
	SkillSword();

	float Getpower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;
};
