#include "Skillpowerbuff.h"
#include"Player.h"

Skillpowerbuff::Skillpowerbuff(int value):BuffSkill("Buff",4),m_buffValue(value)
{
}



void Apply(Player* player)
{
	if (!player)return;
}