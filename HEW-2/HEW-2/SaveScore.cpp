#include "SaveScore.h"

SaveScore::SaveScore() :Scene(SceneType::SaveScore)
{
}

void SaveScore::InitScene()
{
	// ------------------------------------------------
	// ResultSceneから渡された結果データ取得
	// ------------------------------------------------
	ResultData data = Scene::GetResultData();

	bool clear = data.isClear;
	int ResultScore = data.score;

	// PlayWallpaper 背景
	PlayWallpaper = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1670.0f, 940.0f, 0.0f);
	PlayWallpaper->Init("asset/wallpaper.png");
	// ------------------------------------------------
	// 背景はUIではなくワールド描画にする
	// ------------------------------------------------
	PlayWallpaper->SetUI(false);

	// ResultWindow リザルトウィンドウ
	ResultWindow = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(900.0f, 500.0f, 0.0f);
	ResultWindow->Init("asset/resultwindow.png");
	ResultWindow->SetUI(true);

	// TitleWindow　タイトルボタン
	TitleWindow = AddObject()
		->SetPos(300.0f, -200.0f, 0.0f)
		->SetSize(200.0f, 50.0f, 0.0f);
	TitleWindow->Init("asset/titlewindow.png");
	TitleWindow->SetUI(true);

	// PlayerCharacter　キャラクター表示
	PlayerCharacter = AddObject()
		->SetPos(-270.0f, 0.0f, 0.0f)
		->SetSize(220.0f, 250.0f, 0.0f);
	if (clear == true) {
		PlayerCharacter->Init("asset/clearplayer.png", 8, 3);
	}
	else
	{
		PlayerCharacter->Init("asset/overplayer.png", 8, 3);
	}
	PlayerCharacter->SetSpriteSheet(8, 3);
	PlayerCharacter->SetUI(true);


	// ScoreText_Text　ランキング　
	ScoreText_Text = AddObject()
		->SetPos(100.0f, 0.0f, 0.0f)
		->SetSize(500.0f, 300.0f, 0.0f);
	ScoreText_Text->Init("asset/ranking.png");
	ScoreText_Text->SetUI(true);
	// ------------------------------------------------
	// スコア数字生成
	// ------------------------------------------------
	CreateNumberText(200.0f, 100.0f, std::to_string(ResultScore));
	std::cout << "(Debug) SaveScoreScene!" << std::endl;
}

void SaveScore::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE) || Input::GetButtonTrigger(XINPUT_RIGHT_SHOULDER))
	{
		SetNextScene(SceneType::Title);
	}
}

void SaveScore::DrawScene()
{
	// =============================
	// ① ワールド描画
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
void SaveScore::UninitScene()
{
	ClearObject();
}

void SaveScore::CreateNumberText(float startX, float y, std::string text)
{
	// ------------------------------------------------
	// 1文字ごとの間隔
	// ------------------------------------------------
	float spacing = 32.0f; // 1文字進む幅（フォントサイズに合わせて調整）

	// ------------------------------------------------
	// 文字生成
	// ------------------------------------------------
	for (size_t i = 0; i < text.length(); ++i) {
		float nU = 0.0f;

		// -------------------------------
		// 数字判定
		// -------------------------------
		if (text[i] >= '0' && text[i] <= '9')
		{
			nU = (float)(text[i] - '0');
		}

		// -------------------------------
		// Object生成
		// -------------------------------
		Object* obj = AddObject()
			->SetPos(startX + (i * spacing) + 40.0f, y - 50.0f, 0.0f)
			->SetSize(32.0f, 32.0f, 0.0f);

		// -------------------------------
		// スプライト設定
		// -------------------------------
		obj->Init("asset/scoretext.png", 10, 1);
		obj->SetSpriteSheet(10, 1);

		// -------------------------------
		// 表示する数字
		// -------------------------------
		obj->numU = nU;

		// -------------------------------
		// UI描画
		// -------------------------------
		obj->SetUI(true);
	}
}