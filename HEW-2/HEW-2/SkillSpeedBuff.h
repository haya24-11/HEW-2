#pragma once
#include "BuffSkill.h"

class SkillSpeedBuff : public BuffSkill {
private:
    int m_skilllLevel = 0;
public:
    SkillSpeedBuff();
    // プレイヤーの移動速度(moveSpeed)を強化する
    void Apply(class Player* player) override;
    const char* GetIconPath() const override;
};