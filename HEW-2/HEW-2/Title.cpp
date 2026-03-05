#include "Title.h"
#include "Game.h"
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

Title::Title() : Scene(SceneType::Title)
{
}

void Title::InitScene()
{
	std::cout << "(Debug) TitleScene!" << std::endl;

	// =========================
	// タイトル背景
	// =========================
	TitleBackground = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1670.0f, 940.0f, 0.0f)
		->SetAngle(0.0f);
	TitleBackground->Init("asset/Title/titlebackground.png");
	TitleBackground->SetUI(true);

	// =========================
	// 先読み
	// =========================
	PreloadTexture(g_pDevice, "asset/Texture/enemy_slime.png");
	PreloadTexture(g_pDevice, "asset/Texture/enemy_slime_blue.png");
	PreloadTexture(g_pDevice, "asset/Texture/enemy_slime_red.png");

	// =========================
	// スライム 9体（種類ごとに3体）
	// =========================
	const float baseX = -1200.0f;     // 左の出現基準
	const float xStep = 30.0f;        // 3体のズラし
	const float size = 90.0f;

	// ---- 緑 3体 ----
	for (int i = 0; i < 3; ++i)
	{
		m_green[i].obj = AddObject();
		m_green[i].obj->Init("asset/Texture/enemy_slime.png", 8, 4);
		m_green[i].obj->SetSpriteSheet(8, 4);
		m_green[i].obj->SetSize(size, size, 0.0f);

		m_green[i].y = -340.0f + (i - 1) * 20.0f;             // ちょい上下ずらし
		m_green[i].respawnX = baseX + i * xStep;              // ✅ 毎回このXに戻す
		m_green[i].obj->SetPos(m_green[i].respawnX, m_green[i].y, 0.0f);

		m_green[i].speed = 20.0f + i * 2.0f;                  // 微妙に速度差
		m_green[i].dir = 1.0f;
		m_green[i].anim = { 0, 31, 0.10f, true };             // 0～31の32枚
		m_green[i].obj->numU = 0;
		m_green[i].obj->numV = 0;

		m_green[i].baseY = m_green[i].y;

		m_green[i].waveAmp = 10.0f + i * 3.0f;     
		m_green[i].waveSpd = 1.4f + i * 0.25f;     
		m_green[i].waveT = (float)i * 0.7f;        

		m_green[i].baseSpeed = 18.0f + i * 2.0f;   
		m_green[i].spdAmp = 6.0f;                  
		m_green[i].spdSpd = 1.1f + i * 0.2f;       
		m_green[i].spdT = (float)i * 0.9f;

		m_green[i].speed = m_green[i].baseSpeed;   
	}

	// ---- 青 3体 ----
	for (int i = 0; i < 3; ++i)
	{
		m_blue[i].obj = AddObject();
		m_blue[i].obj->Init("asset/Texture/enemy_slime_blue.png", 8, 4);
		m_blue[i].obj->SetSpriteSheet(8, 4);
		m_blue[i].obj->SetSize(size, size, 0.0f);

		m_blue[i].y = -320.0f + (i - 1) * 20.0f;
		m_blue[i].respawnX = (baseX + 100.0f) + i * xStep;    // 緑より少し右から
		m_blue[i].obj->SetPos(m_blue[i].respawnX, m_blue[i].y, 0.0f);

		m_blue[i].speed = 20.0f + i * 2.0f;
		m_blue[i].dir = 1.0f;
		m_blue[i].anim = { 0, 31, 0.10f, true };
		m_blue[i].obj->numU = 0;
		m_blue[i].obj->numV = 0;
		m_blue[i].baseY = m_blue[i].y;

		m_blue[i].waveAmp = 10.0f + i * 3.0f;
		m_blue[i].waveSpd = 1.4f + i * 0.25f;
		m_blue[i].waveT = (float)i * 0.7f;

		m_blue[i].baseSpeed = 18.0f + i * 2.0f;
		m_blue[i].spdAmp = 6.0f;
		m_blue[i].spdSpd = 1.1f + i * 0.2f;
		m_blue[i].spdT = (float)i * 0.9f;

		m_blue[i].speed = m_blue[i].baseSpeed;
	}

	// ---- 赤 3体 ----
	for (int i = 0; i < 3; ++i)
	{
		m_red[i].obj = AddObject();
		m_red[i].obj->Init("asset/Texture/enemy_slime_red.png", 8, 4);
		m_red[i].obj->SetSpriteSheet(8, 4);
		m_red[i].obj->SetSize(size, size, 0.0f);

		m_red[i].y = -360.0f + (i - 1) * 20.0f;
		m_red[i].respawnX = (baseX -250.0f) + i * xStep;
		m_red[i].obj->SetPos(m_red[i].respawnX, m_red[i].y, 0.0f);

		m_red[i].speed = 20.0f + i * 2.0f;
		m_red[i].dir = 1.0f;
		m_red[i].anim = { 0, 31, 0.10f, true };
		m_red[i].obj->numU = 0;
		m_red[i].obj->numV = 0;

		m_red[i].baseY = m_red[i].y;
		m_red[i].waveAmp = 10.0f + i * 3.0f;
		m_red[i].waveSpd = 1.4f + i * 0.25f;
		m_red[i].waveT = (float)i * 0.7f;

		m_red[i].baseSpeed = 18.0f + i * 2.0f;
		m_red[i].spdAmp = 6.0f;
		m_red[i].spdSpd = 1.1f + i * 0.2f;
		m_red[i].spdT = (float)i * 0.9f;

		m_red[i].speed = m_red[i].baseSpeed;
	}

	// =========================
	// バックタイトルロゴ
	// =========================
	title_backlogo = AddObject()
		->SetPos(0.0f, 000.0f, 0.0f)
		->SetSize(1550.0f, 950.0f, 0.0f)
		->SetAngle(0.0f);
	title_backlogo->Init("asset/Title/title_backlogo.png");
	title_backlogo->SetUI(true);

	// 背景はワールド描画にする
	TitleBackground->SetUI(false);
	// 背景ロゴ
	TitleBackLogo = AddObject();
	TitleBackLogo->Init("asset/Title/title_backlogo.png");
	TitleBackLogo->SetPos(0, 0, 0);
	TitleBackLogo->SetSize(1674.0f, 940.0f, 0.0f);
	TitleBackLogo->SetUI(true);

	// ★中心位置を保存
	m_logoCenterX = 0.0f;

	// タイトルロゴ
	Object* TitleLogo = AddObject();
	TitleLogo->Init("asset/Title/title_logo.png");
	TitleLogo->SetPos(-65,145, 0);
	TitleLogo->SetSize(891.3f, 651.0f, 0.0f);      // ★ここでサイズ調整
	TitleLogo->SetUI(true);
	// =========================
	// ™マーク
	// =========================
	TitleTM = AddObject();
	TitleTM->Init("asset/Title/title_tm.png"); // ™画像
	TitleTM->SetPos(320, -87, 0);              // ←ロゴ右上あたりに配置
	TitleTM->SetSize(75.0f, 45.0f, 0.0f);      // サイズは好みで
	TitleTM->SetUI(true);
	// =========================
	// タイトルロゴ
	// =========================

	TitleLogo = AddObject()
		->SetPos(-30.0f, 150.0f, 0.0f)
		->SetSize(800.0f, 650.0f, 0.0f)
		->SetAngle(0.0f);
	TitleLogo->Init("asset/Title/title_logo.png");
	TitleLogo->SetUI(true);


	// =========================
	// SCORE（スプライトシート 2枚）
	// =========================
	ScoreLogo = AddObject()
		->SetPos(20.0f, -330.0f, 0.0f)
		->SetSize(280.0f, 85.0f, 0.0f)
		->SetAngle(0.0f);
	ScoreLogo->Init("asset/Title/score.png", 2, 1);
	ScoreLogo->SetUI(true);
	// =========================
	// GAME START（スプライトシート 2枚）
	// =========================
	GameStartLogo = AddObject()
		->SetPos(20.0f, -210.0f, 0.0f)
		->SetSize(410.0f, 85.0f, 0.0f)
		->SetAngle(0.0f);
	GameStartLogo->SetUI(true);
	GameStartLogo->Init("asset/Title/gamestart.png", 2, 1);

	// =========================
	// EXIT（スプライトシート 2枚）
	// =========================
	ExitLogo = AddObject()
		->SetPos(720.0f, -430.0f, 0.0f)
		->SetSize(180.0f, 85.0f, 0.0f)
		->SetAngle(0.0f);
	ExitLogo->Init("asset/Title/exit.png", 2, 1);
	ExitLogo->SetUI(true);
	// =========================
	// スプライトシート設定（2×1）
	// =========================




	
}
void Title::UpdateScene(float deltaTime)
{
	if (deltaTime > 0.05f) deltaTime = 0.05f; 
	m_count++;

	// =========================
	// タイトルバックロゴ揺れ
	// =========================

	// 時間加算
	m_logoTime++;

	// 無限に増えるの防止
	if (m_logoTime > 1000.0f)
	{
		m_logoTime = 0.0f;
	}

	// 1往復の時間（秒）
	float period = 200.0f;

	// メトロノーム角度
	float t = m_logoTime * DirectX::XM_2PI / period;

	// 振れ角
	float maxAngle = 6.0f;

	float angle = sinf(t) * maxAngle;

	// 位置は固定
	TitleBackLogo->SetPos(
		m_logoCenterX,
		0.0f,
		0.0f
	);
	TitleBackLogo->SetAngle(angle);

	// =========================
	// XInput：トリガー（押した瞬間だけ true）
	// =========================
	static WORD prevButtons = 0;

	XINPUT_STATE pad{};
	WORD buttons = 0;
	if (XInputGetState(0, &pad) == ERROR_SUCCESS)
		buttons = pad.Gamepad.wButtons;

	auto PadTrigger = [&](WORD mask) -> bool
		{
			return (buttons & mask) && !(prevButtons & mask);
		};

	// =========================
	// メニュー移動入力（キーボード W/S + パッド D-Pad）
	// =========================
	int move = 0;
	if (Input::GetKeyTrigger('S') || PadTrigger(XINPUT_GAMEPAD_DPAD_DOWN)) move++; // 下へ
	if (Input::GetKeyTrigger('W') || PadTrigger(XINPUT_GAMEPAD_DPAD_UP))   move--; // 上へ

	TitleMenu += move;

	// =========================
	// 範囲制限（1～3）
	// =========================
	if (TitleMenu < 1) TitleMenu = 1;
	if (TitleMenu > 3) TitleMenu = 3;

	// =========================
	// 決定入力（キーボード + パッド）
	// =========================
	bool decide =
		Input::GetKeyTrigger(VK_SPACE)
		|| PadTrigger(XINPUT_GAMEPAD_A)
		|| PadTrigger(XINPUT_GAMEPAD_START);

	// =========================
	// 選択中メニューの点滅（numU: 0/1 切り替え）
	// 他のメニューは numU=0 に戻す
	// =========================
	switch (TitleMenu)
	{
	case 1: // GAME START
		if (decide)
			SetNextScene(SceneType::GamePlay);

		if (m_count >= 30)
		{
			GameStartLogo->numU = (GameStartLogo->numU == 0) ? 1 : 0;
			m_count = 0;
		}
		ScoreLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 2: // SCORE
		if (decide)
			SetNextScene(SceneType::Result);

		if (m_count >= 30)
		{
			ScoreLogo->numU = (ScoreLogo->numU == 0) ? 1 : 0;
			m_count = 0;
		}
		GameStartLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 3: // EXIT
		if (decide)
			PostQuitMessage(0); // ゲーム終了（ウィンドウ閉じる）

		if (m_count >= 30)
		{
			ExitLogo->numU = (ExitLogo->numU == 0) ? 1 : 0;
			m_count = 0;
		}
		GameStartLogo->numU = 0;
		ScoreLogo->numU = 0;
		break;
	}

	// =========================
	// ✅ スライム歩行演出（毎フレーム更新）
	// =========================
	for (int i = 0; i < 3; ++i) UpdateSlimeWalker(m_green[i], deltaTime);
	for (int i = 0; i < 3; ++i) UpdateSlimeWalker(m_blue[i], deltaTime);
	for (int i = 0; i < 3; ++i) UpdateSlimeWalker(m_red[i], deltaTime);

	// 次フレーム用に保持
	prevButtons = buttons;
}
void Title::DrawScene()
{
	// =============================
	// ① ワールド描画（UI以外）
	// =============================
	for (auto& obj : objects)
	{
		// =========================
		// メニューUI（2x1）
		// =========================
		if (obj.get() == GameStartLogo) { obj->Draw((int)GameStartLogo->numU); continue; }
		if (obj.get() == ScoreLogo) { obj->Draw((int)ScoreLogo->numU);     continue; }
		if (obj.get() == ExitLogo) { obj->Draw((int)ExitLogo->numU);      continue; }

		// =========================
		// ✅ スライムは Draw(frameIndex) で描画（FlipX を反映させる）
		// =========================
		bool drewSlime = false;

		// 緑3
		for (int i = 0; i < 3 && !drewSlime; ++i)
		{
			if (obj.get() == m_green[i].obj)
			{
				const int frame = (int)(m_green[i].obj->numV) * 8 + (int)(m_green[i].obj->numU);
				obj->Draw(frame);
				drewSlime = true;
			}
		}
		// 青3
		for (int i = 0; i < 3 && !drewSlime; ++i)
		{
			if (obj.get() == m_blue[i].obj)
			{
				const int frame = (int)(m_blue[i].obj->numV) * 8 + (int)(m_blue[i].obj->numU);
				obj->Draw(frame);
				drewSlime = true;
			}
		}
		// 赤3
		for (int i = 0; i < 3 && !drewSlime; ++i)
		{
			if (obj.get() == m_red[i].obj)
			{
				const int frame = (int)(m_red[i].obj->numV) * 8 + (int)(m_red[i].obj->numU);
				obj->Draw(frame);
				drewSlime = true;
			}
		}

		if (drewSlime) continue;

		// =========================
		// その他（通常描画）
		// =========================
		if (obj->IsUI()) continue;
		obj->Draw();
	}

	// =============================
	// ② UI描画
	// =============================
	for (auto& obj : objects)
	{
		if (!obj->IsUI()) continue;
		obj->Draw();
	}
}

void Title::UninitScene()
{
	// 必要ならここで解放処理
}


void Title::UpdateSlimeWalker(SlimeWalker& s, float dt)
{
	if (!s.obj) return;

	// ---- アニメ更新 ----
	s.frameTimer += dt;
	if (s.frameTimer >= s.anim.sec)
	{
		while (s.frameTimer >= s.anim.sec) s.frameTimer -= s.anim.sec;

		s.frame++;
		if (s.frame >= s.anim.count) s.frame = 0;

		const int idx = s.anim.start + s.frame; // 0..31
		s.obj->numU = (float)(idx % 8);
		s.obj->numV = (float)((idx / 8) % 4);
	}

	// ---- 速度ゆらぎ（前後に混ざる）----
	s.spdT += dt;
	const float curSpeed = s.baseSpeed + sinf(s.spdT * s.spdSpd) * s.spdAmp;

	// ---- 移動 ----
	auto p = s.obj->GetPos();
	p.x += curSpeed * s.dir * dt;

	// ---- 上下ウェーブ（上下に混ざる）----
	s.waveT += dt;
	p.y = s.baseY + sinf(s.waveT * s.waveSpd) * s.waveAmp;

	s.obj->SetPos(p.x, p.y, p.z);

	// ---- 向き（ゲームプレイ方式：FlipX）----
	s.obj->SetFlipX(s.dir > 0.0f);

	// ---- ループ（毎回同じ開始位置へ）----
	if (p.x > s.xMax)
	{
		s.obj->SetPos(s.respawnX, s.baseY, p.z);
		s.frame = 0;
		s.frameTimer = 0.0f;
		s.obj->numU = 0;
		s.obj->numV = 0;

		// ✅ ここもリセットすると挙動が安定する
		s.waveT = 0.0f;
		s.spdT = 0.0f;
	}
}