#pragma once
#include <vector>
#include "Mode.h"
#include "Combo.h"

class ModeManager
{
public:
    void SwitchMode(const Mode& mode);

    const Mode& GetCurrentMode() const;

    std::vector<Skill> GetAvailableSkills() const;

    void AddComboBonus();
    void ResetCombo();

private:
    Mode currentMode;
    Combo combo;
};
