#pragma once
#include "Skill.h"
class Skillpowerbuff :public Skill
{
private:

	int m_buffValue;//UŒ‚—Íã¸—Ê

public:

	void Apply(Player* player)override;


	Skillpowerbuff(int value);


};

