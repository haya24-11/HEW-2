#include "Skillpowerbuff.h"
#include"Player.h"

Skillpowerbuff::Skillpowerbuff(int value):BuffSkill("Buff",4),m_buffValue(value)
{
}

void Skillpowerbuff::Apply(Player* player)
{

	if (!player)return;

	int atk = player->GetPower();
	atk = static_cast<int>(atk * 1.2);
	player->SetPower(atk);
	printf("Npw Power -> Atk %d\n", player->GetPower());

}

const char* Skillpowerbuff::GetIconPath() const
{

	return "asset/Texture/skill_buff_default.png";
	
}
