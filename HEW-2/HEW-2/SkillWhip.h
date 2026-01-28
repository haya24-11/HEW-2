#pragma once
#include"WeaponSkills.h"

class SkillWhip :public WeaponSkills
{

private:


	float whipPower;
	DirectX::SimpleMath::Vector2 whipArea;


public:
	SkillWhip();

	float GetPower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;
};
