#include "Skillpowerbuff.h"
#include "Player.h"

Skillpowerbuff::Skillpowerbuff(int value) : BuffSkill("Buff", 4), m_buffValue(value)
{
}

void Skillpowerbuff::Apply(Player* player)
{
    if (!player) return;
    m_skilllLevel++;
    int currentP = player->GetPower(); // Chara::power を取得
    switch (m_skilllLevel) {
    case 1: currentP = (int)(currentP * 1.2); break;
    case 2: currentP = (int)(currentP * 1.5); break;
    case 3: currentP = (int)(currentP * 1.6); break;
    default: currentP += 5; break;
    }
    player->SetPower(currentP);

    float kb = player->GetHeavyDamageMul();
    switch (m_skilllLevel) {
    case 1: kb *= 1.3f; break;
    case 2: kb *= 1.6f; break;
    case 3: kb *= 2.0f; break;
    default: kb *= 1.1f; break;
    }
    player->SetHeavyDamageMul(kb);
    printf("[DEBUG] KB Level: %d, New KB: %.1f\n", m_skilllLevel, kb);
    // 2. ログを出して現在のレベルと攻撃力を確認
    printf("[DEBUG] Skill Level: %d, Current Atk: %d\n", m_skilllLevel, currentP);

    // 5. 最終結果を表示
    printf("[DEBUG] New Power -> Atk %d\n", player->GetPower());
}

const char* Skillpowerbuff::GetIconPath() const
{
    switch (m_skilllLevel) {
    case 0: return "asset/UI/AttackBuff_lv1.png";
    case 1: return "asset/UI/AttackBuff_lv2.png";
    case 2: return "asset/UI/AttackBuff_lv3.png";
    default: break;
    }
	
}
