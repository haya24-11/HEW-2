#pragma once
#include "BuffSkill.h"

class SkillComboBuff : public BuffSkill {
private:
    int m_skilllLevel = 0;

public:
    SkillComboBuff();
    void Apply(class Player* player) override;
    const char* GetIconPath() const override;
};
