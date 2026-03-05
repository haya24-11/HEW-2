#include "SkillComboBuff.h"
#include "Player.h"
#include <cstdio>

SkillComboBuff::SkillComboBuff() : BuffSkill("ComboUp", 2) {
    m_skilllLevel = 0;
}

void SkillComboBuff::Apply(Player* player) {
   /* if (!player) return;
    m_skilllLevel++;

    float ratio = player->GetExpRatio();
    switch (m_skilllLevel) {
    case 1: ratio += 0.2f; break;
    case 2: ratio += 0.4f; break;
    case 3: ratio += 0.6f; break;
    default: ratio += 0.1f; break;
    }
    player->SetExpRatio(ratio);
    printf("[DEBUG] Combo Level: %d, Exp Ratio: %.1f\n", m_skilllLevel, ratio);*/
}

const char* SkillComboBuff::GetIconPath() const {
    return "asset/UI/buff_combo.png";
}
