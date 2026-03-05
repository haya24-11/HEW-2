#pragma once
#include "BuffSkill.h"
#include"Player.h"
class Skillpowerbuff :public BuffSkill
{
private:
	int m_skilllLevel = 0;

	int m_buffValue;//UŒ‚—Íã¸—Ê
	int m_AtkBufflv = 0;

public:
	Skillpowerbuff(int value);

	void Apply(Player* player) override;
	const char* GetIconPath() const override;
};