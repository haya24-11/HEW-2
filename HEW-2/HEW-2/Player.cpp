#include "Player.h"
#include "Skill.h"

Player::Player()
{
	hp = 100;
	power = 10;

	moveSpeed = 50.0f; // Player 固有の速さ

	// ===== Animation定義 =====
	// 待機（idle.png）
	m_idleAnim = { 0, 30, 0.15f, true };

	// 移動（walk.png）
	m_walkAnim = { 0, 8, 0.1f, true };
}

void Player::Update(float deltaTime)
{
	printf("[Player] dt=%.3f\n", deltaTime);
	// ===== 入力 =====
	auto dir = GetMoveInput();
	printf("dir = (%.1f, %.1f)\n", dir.x, dir.y);
	bool isMoving = (dir.LengthSquared() > 0.01f);

	// ===== 向き更新（入力がある時だけ）=====
	if (dir.x > 0.0f)
	{
		m_facingRight = true;
	}
	else if (dir.x < 0.0f)
	{
		m_facingRight = false;
	}

	// ===== 移動 =====
	auto before = GetPos();
	Move(dir, deltaTime);
	auto after = GetPos();
	// デバッグ用
	printf("pos before(%.1f, %.1f) after(%.1f, %.1f)\n",
		before.x, before.y, after.x, after.y);

	if (isMoving && m_state != State::Walk)
	{
		m_state = State::Walk;
		m_object->SetTexture("asset/Texture/player_walk.png");
		m_object->SetSpriteSheet(3, 3);
		m_animator.Play(m_walkAnim);
	}
	else if (!isMoving && m_state != State::Idle)
	{
		m_state = State::Idle;
		m_object->SetTexture("asset/Texture/player_idle.png");
		m_object->SetSpriteSheet(6, 6);
		m_animator.Play(m_idleAnim);
	}
	// ===== ★ 向き反映 =====
	/*
		Idle : 原画は右向き
		Walk : 原画は左向き
	*/
	bool textureIsRightFacing = (m_state == State::Idle);
	// 原画の向き と 向きたい方向 が違えば反転
	bool flipX = (textureIsRightFacing != m_facingRight);
	m_object->SetFlipX(flipX);

	// アニメ更新
	m_animator.Update(deltaTime);

	// 生存判定など
	Chara::Update(deltaTime);
}

int Player::GetAnimFrame() const
{
	return m_animator.GetCurrentFrame();
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
