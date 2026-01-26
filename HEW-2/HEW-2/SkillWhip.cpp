#include "SkillWhip.h"

SkillWhip::SkillWhip() :Skill("Whip", 3, SkillType::Weapon), whipPower(100.0f), whipArea(30.0f, 30.0f)
{
}

float SkillWhip::GetPower() const
{
	return whipPower;
}

DirectX::SimpleMath::Vector2 SkillWhip::GetArea() const
{
	return whipArea;
}
