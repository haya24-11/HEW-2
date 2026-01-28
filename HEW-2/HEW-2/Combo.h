#pragma once

/*
    Combo
    =====
    ・コンボ数管理
    ・倍率計算
*/

class Combo
{
public:
    void AddCombo();
    void Reset();
    float GetMultiplier() const;

private:
    int combCount = 0;
    float combBonus = 0.1f;
};