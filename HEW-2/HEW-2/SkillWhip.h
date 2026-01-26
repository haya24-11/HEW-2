#pragma once
#include"Skill.h"

class SkillWhip :public Skill
{

private:


	float whipPower;
	DirectX::SimpleMath::Vector2 whipArea;


public:
	SkillWhip();

	float GetPower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;

};
