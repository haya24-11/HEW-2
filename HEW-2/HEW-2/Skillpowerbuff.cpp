//#include "Skillpowerbuff.h"
//#include"Player.h"
//
//Skillpowerbuff::Skillpowerbuff(int value):BuffSkill("Buff",4),m_buffValue(value)
//{
//}
//
//Player playerExp;
//
//
//static void Apply(Player* player)
//{
//	if (!player)return;
//	int exp = playerExp.GetLevel();
//	switch (exp)
//	{
//	case 2:
//		int atk = player->GetPower();
//		atk = static_cast<int>(atk * 1.2);
//		player->SetPower(atk);
//		break;
//
//	case 3:
//		 atk = player->GetPower();
//		atk = static_cast<int>(atk * 1.2);
//		player->SetPower(atk);
//		break;
//	default:
//		break;
//	}
//	
//}

#include "Skillpowerbuff.h"
#include "Player.h"

Skillpowerbuff::Skillpowerbuff(int value) : BuffSkill("Buff", 4), m_buffValue(value)
{
}

 void Skillpowerbuff::Apply(Player* player)
{
    if (!player) return;
    m_skillbufflevel++;

   // int level = player->GetLevel(); // 同じplayerからレベルを取得

    int atk = player->GetPower();   // case外で宣言してスコープ問題を回避

    switch (m_skillbufflevel)
    {
    case 2:
        atk = static_cast<int>(atk * 1.2);
        player->SetPower(atk);
    case 3:
        atk = static_cast<int>(atk * 1.5);
        player->SetPower(atk);
        break;
    default:
        break;
    };

}