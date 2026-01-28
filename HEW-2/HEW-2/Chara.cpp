#include "Chara.h"

void Chara::Update(float /*deltaTime*/)
{
	// HPが 0 以下なら死亡扱い
	if (hp <= 0)
	{
		isAlive = false;
	}
}

void Chara::Move(const DirectX::SimpleMath::Vector2& direction,float deltaTime)
{
	// 入力がない場合は何もしない
	if (direction.LengthSquared() == 0.0f)
		return;

	/*
	   正規化する理由
	   ・斜め移動(W+Aなど)が速くならないようにするため
   */
	DirectX::SimpleMath::Vector2 dir = direction;
	dir.Normalize();

	/*
		移動量 = 方向 × 速度 × 経過時間
		→ フレームレート非依存
	*/
	position += dir * moveSpeed * deltaTime;
}

int Chara::GetHp()const
{
	return hp;
}