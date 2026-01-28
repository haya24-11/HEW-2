#include "Skillpowerbuff.h"
#include"Player.h"




Skillpowerbuff::Skillpowerbuff(int value):Skill("Buff",4,SkillType::Buff),m_buffValue(value)
{

}


void Skillpowerbuff::Apply(Player* player)	
{
	if (!player)return;

	int power = player->GetAttck();		//　20％アップ
	int m_buffValue=static_cast<int > (power*0.2);//playerの攻撃力を割合アップ
	player->SetPower(m_buffValue);
}