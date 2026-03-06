#pragma once
#include "BuffSkill.h"

class SkillSpecialBuff : public BuffSkill {
public:
    SkillSpecialBuff();
    void Apply(class Player* player) override;
    const char* GetIconPath() const override;

    static bool IsAlreadyApplied() { return m_isApplied; }

private:
    static bool m_isApplied; // 1‰ñŒÀ’èƒtƒ‰ƒO
    int m_skilllLevel = 0;
};
