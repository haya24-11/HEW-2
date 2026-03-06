#include "SkillSpeedBuff.h"
#include "Player.h"
#include <cstdio>

SkillSpeedBuff::SkillSpeedBuff() : BuffSkill("SpeedUp", 104) {
    m_skilllLevel = 0; // ゴミデータ防止
}

void SkillSpeedBuff::Apply(Player* player) {
    if (!player) return;
    m_skilllLevel++;

    // 基底クラス(Chara)の moveSpeed を取得
    float s = player->GetMoveSpeed();

    // レベルに応じて移動速度を乗算で強化
    switch (m_skilllLevel) {
    case 1: s *= 1.15f; break; // 15% アップ
    case 2: s *= 1.30f; break; // 30% アップ
    case 3: s *= 1.50f; break; // 50% アップ
    default: s *= 1.05f; break; // それ以降は微増
    }

    player->SetMoveSpeed(s);

    printf("[SKILL] MoveSpeed Up! Level: %d, New Speed: %.1f\n",
        m_skilllLevel, player->GetMoveSpeed());
}

const char* SkillSpeedBuff::GetIconPath() const {
    switch (m_skilllLevel) {
    case 0:return "asset/UI/SpeedBuff_lv1.png";
    case 1:return "asset/UI/SpeedBuff_lv2.png";
    case 2:return "asset/UI/SpeedBuff_lv3.png";
    default: break;
    }
}