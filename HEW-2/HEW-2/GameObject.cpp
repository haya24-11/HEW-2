#include "GameObject.h"

Vector2 GameObject::GetPosition() const
{
    return position;
}

void GameObject::SetPosition(const Vector2& pos)
{
    position = pos;
}