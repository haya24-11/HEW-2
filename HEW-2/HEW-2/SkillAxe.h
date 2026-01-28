#pragma once
#include"WeaponSkills.h"

class SkillAxe:public WeaponSkills
{

private:


	float axePower;
	DirectX::SimpleMath::Vector2 aexArea;


public:
	SkillAxe();

	float GetPower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;

};
