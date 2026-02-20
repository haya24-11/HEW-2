#pragma once

/*
    Exp
    ===
    ・基礎経験値倍率管理
    ・最終獲得経験値計算
*/

class Exp
{
public:
    int CalculateGain(int mutiplier);

protected:
    int expBase = 1;
};

