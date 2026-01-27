#pragma once
#include "Mode.h"

class AMode : public Mode
{
    // Mode‚²‚Æ‚Ì“Áêƒ‹[ƒ‹
private:
   int weakAtk = 2;
   int strongAtkMin = 3;

public:
    AMode();
    int WeakAttack(int BaseAtk);
    int StrongAttakc(int BaseAtk,float pushTime);
    int ComboDamage();
};