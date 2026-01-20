#include "Chara.h"

void Chara::Update(float /*deltaTime*/)
{
	// HP‚ª 0 ˆÈ‰º‚È‚çŽ€–Sˆµ‚¢
	if (hp <= 0)
	{
		isAlive = false;
	}
}

void Chara::Move(const DirectX::SimpleMath::Vector2& direction)
{
	// ’Pƒ‚ÈÀ•WˆÚ“®
	position += direction;
}

int Chara::GetHp()const
{
	return hp;
}