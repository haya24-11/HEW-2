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
	/*
	TitleLogo = AddObject()
		->SetPos(-30.0f, 200.0f, 0.0f)
		->SetSize(550.0f, 450.0f, 0.0f)
		->SetAngle(0.0f);
	TitleLogo->Init("asset/titlelogo.png");
	TitleLogo->SetUI(true);
	*/
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
	GameStartLogo->SetSpriteSheet(2, 1);
	ScoreLogo->SetSpriteSheet(2, 1);
	ExitLogo->SetSpriteSheet(2, 1);
}

void Title::UpdateScene(float deltaTime)
{
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
