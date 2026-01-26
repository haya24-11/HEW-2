#pragma once
#include"Skill.h"

class SkillAxe:public Skill
{

private:


	float axePower;
	DirectX::SimpleMath::Vector2 aexArea;


public:
	SkillAxe();

	float GetPower() const;
	DirectX::SimpleMath::Vector2 GetArea() const;

};
