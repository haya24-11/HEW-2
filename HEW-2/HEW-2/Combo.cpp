#include "Combo.h"

void Combo::AddCombo()
{
	comboCount++;
}

void Combo::Reset()
{
	comboCount = 0;
}

float Combo::GetCount() const
{
	return comboCount;
}