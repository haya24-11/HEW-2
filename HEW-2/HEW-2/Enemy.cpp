#include "Enemy.h"

Enemy::Enemy(){}

void Enemy::Update(float deltaTime)
{
	Chara::Update(deltaTime);
}

void Enemy::Attack()
{
    // AI / s“®Œˆ’è‚Í•Ê’S“–
}

void Enemy::TakeDamage(int damage)
{
    hp -= damage;
}

void Enemy::KnockBack(const DirectX::SimpleMath::Vector2& force)
{
    position += force;
}