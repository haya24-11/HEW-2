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
    float GetCount() const;

private:
    int comboCount = 0;
    float combBonus = 0.3f;
};