#pragma once
#include "BuffSkill.h"
#include"Player.h"
class Skillpowerbuff :public BuffSkill
{
private:

	int m_buffValue;//UŒ‚—Íã¸—Ê

public:

	Skillpowerbuff(int value);

	void Apply(Player* player);

};