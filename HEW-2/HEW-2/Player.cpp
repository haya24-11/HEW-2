#include "Player.h"
#include "Skill.h"

Player::Player()
{
	hp = 100;
	power = 10;
}

void Player::Update(float deltaTime)
{
	// Chara の基本更新処理
	Chara::Update(deltaTime);
}

void Player::Attack()
{
	// 実際の攻撃内容は Mode / Skill 側が決める
}

void Player::ApplyAbility(Skill* skill)
{
	if (!skill) return;

	// 所持スキルとして登録
	skills.push_back(skill);

	// スキの効果を Player に適用
	//skill->Apply(this); todo
}
