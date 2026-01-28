#include "AMode.h"



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
	return;
}
