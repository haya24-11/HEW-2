#include "ModeManager.h"
#include "Player.h"
#include "Skill.h"

void ModeManager::SwitchMode(Mode* mode)
{
    currentMode = mode;
}

std::vector<Skill*> ModeManager::GetAvailableSkills(const Player& player) const
{
    std::vector<Skill*> result;

    for (auto s : player.GetLearnedSkills())
    {
        if (currentMode && currentMode->CanUseSkill(*s))
            result.push_back(s);
    }
    return result;
}

int ModeManager::CalculateFinalExp(int baseExp) const
{
    if (!currentMode) return baseExp;

    float comboMul = currentMode->CalculateComboMultiplier(combo.GetCount());

    return static_cast<int>(baseExp * comboMul);

}
