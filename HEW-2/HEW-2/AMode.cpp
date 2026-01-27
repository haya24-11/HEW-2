#include "AMode.h"

AMode::AMode()
{
}

int AMode::WeakAttack(int BaseAtk)
{
	return BaseAtk* weakAtk;
}

int AMode::StrongAttakc(int BaseAtk,float pushTime)
{
	return BaseAtk * strongAtkMin * pushTime;
}

int AMode::ComboDamage()
{
	return combo.GetMultiplier();
}
