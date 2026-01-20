#include "GameObject.h"

const DirectX::SimpleMath::Vector2& GameObject::GetPosition()const
{
	return position;
}

void GameObject::SetPosition(const DirectX::SimpleMath::Vector2& pos)
{
	position = pos;
}

bool GameObject::IsAlive()const
{
	return isAlive;
}


