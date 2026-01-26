#include "SkillAxe.h"

SkillAxe::SkillAxe():Skill("Aex", 2, SkillType::Weapon), axePower(100.0f),aexArea(30.0f,30.0f)
{
}

float SkillAxe::GetPower() const
{
	return axePower;
}

DirectX::SimpleMath::Vector2 SkillAxe::GetArea() const
{
	return aexArea ;
}
