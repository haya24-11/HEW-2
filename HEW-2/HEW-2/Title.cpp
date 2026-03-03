#include "Title.h"
#include "Game.h"
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

Object* obj = new Object;

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
	TitleBackground->Init("asset/titlebackground.png");
	TitleBackground->SetUI(true);
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
		->SetPos(20.0f, -360.0f, 0.0f)
		->SetSize(250.0f, 110.0f, 0.0f)
		->SetAngle(0.0f);
	ScoreLogo->Init("asset/score.png", 2, 1);
	ScoreLogo->SetUI(true);
	// =========================
	// GAME START（スプライトシート 2枚）
	// =========================
	GameStartLogo = AddObject()
		->SetPos(20.0f, -210.0f, 0.0f)
		->SetSize(370.0f, 110.0f, 0.0f)
		->SetAngle(0.0f);
	GameStartLogo->SetUI(true);
	GameStartLogo->Init("asset/gamestart.png", 2, 1);

	// =========================
	// EXIT（スプライトシート 2枚）
	// =========================
	ExitLogo = AddObject()
		->SetPos(720.0f, -400.0f, 0.0f)
		->SetSize(200.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	ExitLogo->Init("asset/exit.png", 2, 1);
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
	for (auto& obj : objects)
	{
		// =========================
		// スプライトシートのフレーム指定描画
		// numU が 0/1 で切り替わる
		// =========================
		if (obj.get() == GameStartLogo) obj->Draw(GameStartLogo->numU);
		else if (obj.get() == ScoreLogo) obj->Draw(ScoreLogo->numU);
		else if (obj.get() == ExitLogo) obj->Draw(ExitLogo->numU);
		else obj->Draw();
	}
}

void Title::UninitScene()
{
	// 必要ならここで解放処理
}
