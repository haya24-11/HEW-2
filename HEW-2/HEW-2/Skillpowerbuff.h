#pragma once
#include "BuffSkill.h"
#include"Player.h"
class Skillpowerbuff :public BuffSkill
{
private:

	int m_buffValue;//UŒ‚—Íã¸—Ê
	
	int m_skilllLevel;
public:

	Skillpowerbuff(int value);

	void Apply(Player* player);

};