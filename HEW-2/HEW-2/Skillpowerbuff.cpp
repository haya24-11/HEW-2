#include "Skillpowerbuff.h"
#include "Player.h"

Skillpowerbuff::Skillpowerbuff(int value) : BuffSkill("Buff", 4), m_buffValue(value)
{
}

 void Skillpowerbuff::Apply(Player* player)
{
    if (!player) return;
  
    m_skilllLevel++;

   // int level = player->GetLevel(); // 同じplayerからレベルを取得

    int atk = player->GetPower();   // case外で宣言してスコープ問題を回避

    switch (m_skilllLevel)
    {
    case 2:
        atk = static_cast<int>(atk * 1.2);
        player->SetPower(atk);
        break;
    case 3:
        atk = static_cast<int>(atk * 1.5);
        player->SetPower(atk);
        break;
    default:
        break;
    };

}