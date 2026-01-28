#include "Player.h"
#include "Skill.h"
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
Player::Player()
{
	hp = 100;
	power = 10;

	moveSpeed = 15.0f; // Player 固有の速さ

	// ===== Animation定義 =====
	// 待機（idle.png）
	m_idleAnim = { 0, 30, 0.15f, true };

	// 移動（walk.png）
	m_walkAnim = { 0, 8, 0.5f, true };

	// 弱攻撃（Attack.png)
	m_attackLightAnim = { 0,15,0.17f,false };

	// 強攻撃
	m_attackHeavyAnim = { 0,27,0.2f,false };
}

void Player::Update(float deltaTime)
{
	bool attackLightInput = Input::GetKeyTrigger(VK_RETURN); // 押した瞬間
	bool attackHeavyInput =
		(GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
		Input::GetKeyTrigger(VK_RETURN);

	// ===== Attack中の処理 =====
	if (m_state == State::AttackLight || m_state == State::AttackHeavy)
	{
		if (m_animator.IsFinished())
		{
			m_state = State::Idle;
			m_object->SetTexture("asset/Texture/player_idle.png");
			m_object->SetSpriteSheet(6, 6);
			m_animator.Play(m_idleAnim);
		}

		// 向き反映
		bool textureIsRightFacing = true; // idle/attackは右向き原画
		bool flipX = (textureIsRightFacing != m_facingRight);
		m_object->SetFlipX(flipX);

		m_animator.Update(deltaTime);
		Chara::Update(deltaTime);
		return; // ★超重要
	}

	// ===== Attack開始判定 =====
	// ===== 強攻撃（優先）=====
	if (attackHeavyInput)
	{
		m_state = State::AttackHeavy;
		m_object->SetTexture("asset/Texture/player_attack_heavy.png");
		m_object->SetSpriteSheet(6, 5);
		m_animator.Play(m_attackHeavyAnim);
		return;
	}

	// ===== 弱攻撃 =====
	if (attackLightInput)
	{
		m_state = State::AttackLight;
		m_object->SetTexture("asset/Texture/player_attack_light.png");
		m_object->SetSpriteSheet(6, 3);
		m_animator.Play(m_attackLightAnim);
		return; // 即Attackに入る
	}

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

	// =========================
	// (A) ゲームパッド（XInput）- 左スティック
	// =========================
	XINPUT_STATE state{};
	DWORD res = XInputGetState(0, &state); // 0番のコントローラ

	if (res == ERROR_SUCCESS)
	{
		// スティックの生値（-32768 ～ 32767）
		float lx = (float)state.Gamepad.sThumbLX;
		float ly = (float)state.Gamepad.sThumbLY;

		// デッドゾーン（少し触れても動く“ドリフト”防止）
		const float dead = (float)XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

		// デッドゾーン適用
		if (fabsf(lx) < dead) lx = 0.0f;
		if (fabsf(ly) < dead) ly = 0.0f;

		// 正規化（-1 ～ 1）
		const float maxv = 32767.0f;
		dir.x = lx / maxv;
		dir.y = ly / maxv;
	}

	// =========================
	// (B) キーボード（WASD）も併用したい場合
	//     ※パッド優先 + キーボード補助
	// =========================
	if (GetAsyncKeyState('W') & 0x8000) dir.y += 1.0f;
	if (GetAsyncKeyState('S') & 0x8000) dir.y -= 1.0f;
	if (GetAsyncKeyState('A') & 0x8000) dir.x -= 1.0f;
	if (GetAsyncKeyState('D') & 0x8000) dir.x += 1.0f;

	// =========================
	// (C) 長さを正規化（斜め移動でも速度が同じになるように）
	// =========================
	if (dir.LengthSquared() > 1.0f)
		dir.Normalize();

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

int Player::GetPower() const
{
	return power;
}

void Player::Setpower(int value)
{
	power = value;
}
