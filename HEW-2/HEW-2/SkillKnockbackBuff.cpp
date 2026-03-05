#include "SkillKnockbackBuff.h"
#include "Player.h"
#include <cstdio>

SkillKnockbackBuff::SkillKnockbackBuff() : BuffSkill("KnockbackUp", 102) {
    m_skilllLevel = 0; // 初期化（ゴミデータ防止）
}

void SkillKnockbackBuff::Apply(Player* player) {
    if (!player) return;
    m_skilllLevel++;

    // 現在のノックバックパワーを取得（初期値 900.0f 想定）
    float kb = player->GetHeavyKnockBackPower();

    // 🔴 吹き飛ばし力を大幅に強化
    // レベルが上がるほど、敵がピンボールのように超高速で跳ね回るようになります
    switch (m_skilllLevel) {
    case 1: kb *= 1.5f; break; // 1.5倍 (1350)
    case 2: kb *= 2.0f; break; // 2.0倍 (1800)
    case 3: kb *= 3.0f; break; // 3.0倍 (2700) ※画面端まで一瞬で到達
    default: break;
    }

    player->SetHeavyKnockBackPower(kb);

    printf("[SKILL] Knockback Power Up! Lv:%d -> %.1f\n", m_skilllLevel, kb);
}

const char* SkillKnockbackBuff::GetIconPath() const {
    // ノックバック強化用のアイコンパス
    return "asset/UI/buff_knockback.png";
}
