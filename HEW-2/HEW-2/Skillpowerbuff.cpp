#include "Skillpowerbuff.h"
#include"Player.h"

Skillpowerbuff::Skillpowerbuff(int value):BuffSkill("Buff",4),m_buffValue(value)
{
}



static void Apply(Player* player)
{
	if (!player)return;

	int atk = player->GetPower();
	atk = static_cast<int>(atk *0.2);
	player->SetPower(atk);
}