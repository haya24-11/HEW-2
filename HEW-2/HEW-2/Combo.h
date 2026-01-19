#pragma once

class Combo
{
public:
    void AddCombo();
    void Reset();
    float GetMultiplier() const;

private:
    int comboCount = 0;
};