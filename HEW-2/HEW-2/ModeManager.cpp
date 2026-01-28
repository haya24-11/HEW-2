#include "ModeManager.h"

void ModeManager::SwitchMode(const Mode& mode)
{
    currentMode = mode;
}

const Mode& ModeManager::GetCurrentMode() const
{
    return currentMode;
}

std::vector<Skill> ModeManager::GetAvailableSkills() const
{
    return currentMode.GetSkills();
}

void ModeManager::AddComboBonus()
{
    combo.AddCombo();
}

void ModeManager::ResetCombo()
{
    combo.Reset();
}
