#include "Chara.h"

void Chara::Update(float /*deltaTime*/)
{
	// HP‚ª 0 ˆÈ‰º‚È‚çŽ€–Sˆµ‚¢
	if (hp <= 0)
	{
		isAlive = false;
	}
}

void Chara::Move(const DirectX::SimpleMath::Vector2& direction,float deltaTime)
{
	// Object ‚ª–³‚¯‚ê‚Î“®‚©‚¹‚È‚¢
	if (!m_object)
		return;

	// “ü—Í‚ª‚È‚¢ê‡‚Í‰½‚à‚µ‚È‚¢
	if (direction.LengthSquared() == 0.0f)
		return;

	/*
	   ³‹K‰»‚·‚é——R
	   EŽÎ‚ßˆÚ“®(W+A‚È‚Ç)‚ª‘¬‚­‚È‚ç‚È‚¢‚æ‚¤‚É‚·‚é‚½‚ß
   */
	DirectX::SimpleMath::Vector2 dir = direction;
	dir.Normalize();

	// Œ»ÝÀ•W‚ðŽæ“¾
	position = m_object->GetPos();

	// ˆÚ“®—Ê = •ûŒü ~ ‘¬“x ~ Œo‰ßŽžŠÔ
	position.x += dir.x * moveSpeed * deltaTime;
	position.y += dir.y * moveSpeed * deltaTime;

	// Object ‚É”½‰f
	m_object->SetPos(position.x, position.y, position.z);
}

int Chara::GetHp()const
{
	return hp;
}