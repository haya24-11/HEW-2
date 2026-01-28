#pragma once
#include "BuffSkill.h"
class Skillpowerbuff :public BuffSkill
{
private:

	int m_buffValue;//UŒ‚—Íã¸—Ê

public:

	void Apply(Player* player)override;


	Skillpowerbuff(int value);

};