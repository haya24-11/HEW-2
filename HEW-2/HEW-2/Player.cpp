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

	// 入力取得
	DirectX::SimpleMath::Vector2 moveDir = GetMoveInput();

	// 入力があれば移動
	Move(moveDir, deltaTime);
}

DirectX::SimpleMath::Vector2 Player::GetMoveInput() const
{
	DirectX::SimpleMath::Vector2 dir(0.0f, 0.0f);

	/*
	   GetAsyncKeyState
	   ・上位ビットが立っていれば押下中
   */
	if (GetAsyncKeyState('W') & 0x8000)dir.y -= 1.0f;
	if (GetAsyncKeyState('S') & 0x8000)dir.y += 1.0f;
	if (GetAsyncKeyState('A') & 0x8000)dir.x -= 1.0f;
	if (GetAsyncKeyState('D') & 0x8000)dir.x += 1.0f;

	return dir;

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
	//skill->Apply(this);
}
