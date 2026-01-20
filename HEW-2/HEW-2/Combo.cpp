#include "Combo.h"

void Combo::AddCombo()
{
	combCount++;
}

void Combo::Reset()
{
	combCount = 0;
}

float Combo::GetMultiplier() const
{
	return 1.0f + combCount * combBonus;
}
