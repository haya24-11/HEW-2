#pragma once
#include <vector>
#include "Mode.h"
#include "Combo.h"

class Player;

class ModeManager
{
public:
    void SwitchMode(Mode* mode);

    std::vector<Skill*> GetAvailableSkills(const Player& player) const;

    int CalculateFinalExp(int baseExp) const;

private:
    Mode* currentMode = nullptr;
    Combo combo;
};
