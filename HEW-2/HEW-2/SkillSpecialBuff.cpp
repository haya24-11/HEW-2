#include "SkillSpecialBuff.h"
#include "Player.h"
#include <cstdio>

bool SkillSpecialBuff::m_isApplied = false;

SkillSpecialBuff::SkillSpecialBuff() : BuffSkill("RareSpecial", 999) {
    m_skilllLevel = 0;
}

void SkillSpecialBuff::Apply(Player* player) {
    if (!player || m_isApplied) return;

    m_isApplied = true;
    m_skilllLevel = 1;

    // 🔴 全能力（攻撃・速度・ノックバック）を一括で1段階(約20%)強化
    // 攻撃力
    player->SetPower(static_cast<int>(player->GetPower() * 1.2));

    // 移動速度
    player->SetMoveSpeed(player->GetMoveSpeed() * 1.2f);

    // ノックバック威力
    player->SetHeavyKnockBackPower(player->GetHeavyKnockBackPower() * 1.2f);

    printf("[RARE] ★★★ SPECIAL BUFF: ALL STATS EVOLVED! ★★★\n");
}

const char* SkillSpecialBuff::GetIconPath() const {
    return "asset/UI/buff_special.png";
}