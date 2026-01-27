#include "Player.h"
#include "Skill.h"

Player::Player()
{
	hp = 100;
	power = 10;

	moveSpeed = 50.0f; // Player 固有の速さ

	// ===== Animation定義 =====
	// スプライトシート：4×4（16枚）想定

	// 待機（0?30）
	m_idleAnim.startFrame = 0;
	m_idleAnim.frameCount = 30;
	m_idleAnim.frameTime = 0.15f;
	m_idleAnim.loop = true;

	// 歩き（4?9）

	m_walkAnim.startFrame = 4;
	m_walkAnim.frameCount = 6;
	m_walkAnim.frameTime = 0.1f;
	m_walkAnim.loop = true;
}

int Player::GetAnimFrame() const
{
	return m_animator.GetCurrentFrame();
}

void Player::Update(float deltaTime)
{
	if (!m_object) return;
	printf("[Player] dt=%.3f\n", deltaTime);
	// -------------------------
	// 移動入力
	// -------------------------
	auto dir = GetMoveInput();
	bool isMoving = (dir.LengthSquared() > 0.0f);

	// -------------------------
	// アニメーション更新
	// -------------------------
	if (isMoving)
		m_animator.Play(m_walkAnim);
	else
		m_animator.Play(m_idleAnim);

	// アニメ更新
	m_animator.Update(deltaTime);

	// 移動
	Move(dir, deltaTime);

	// 生存判定など
	Chara::Update(deltaTime);
}

DirectX::SimpleMath::Vector2 Player::GetMoveInput() const
{
	DirectX::SimpleMath::Vector2 dir(0.0f, 0.0f);

	/*
	   GetAsyncKeyState
	   ・上位ビットが立っていれば押下中
   */
	if (GetAsyncKeyState('W') & 0x8000)dir.y += 1.0f;
	if (GetAsyncKeyState('S') & 0x8000)dir.y -= 1.0f;
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
