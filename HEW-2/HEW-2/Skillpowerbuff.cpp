#include "Skillpowerbuff.h"
#include"Player.h"

Skillpowerbuff::Skillpowerbuff(int value):Skill("Buff",4,SkillType::Buff),m_buffValue(value)
{
}



void Apply(Player* player)
{
	if (!player)return;
	
}