#pragma once
#include "BuffSkill.h"

class SkillKnockbackBuff : public BuffSkill {
private:
    int m_skilllLevel = 0;
public:
    SkillKnockbackBuff();
    // プレイヤーのノックバック基本値を強化する
    void Apply(class Player* player) override;
    const char* GetIconPath() const override;
};