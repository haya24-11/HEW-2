#include "SaveScore.h"
#include <algorithm>

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
	// =======================================
	// スコア数字オブジェクト生成（最大6桁）
	// =======================================

	// -----------------------------
	// 今回スコア位置
	// -----------------------------
	float startX = -60.0f;
	float y = -190.0f;
	float spacing = 40.0f;

	for (int i = 0; i < 6; i++)
	{
		Object* digit = AddObject()
			->SetPos(startX + (i * spacing), y, 0.0f)
			->SetSize(96.0f, 96.0f, 0.0f);

		digit->Init("asset/scoretext.png", 5, 2);
		digit->SetSpriteSheet(5, 2);
		digit->SetUI(true);

		scoreDigits.push_back(digit);
	}
	int ResultScore = data.score;

	// --------------------------------
	// ランキング更新
	// --------------------------------
	UpdateRanking(ResultScore);

	// --------------------------------
	// 今回のスコア表示
	// --------------------------------
	UpdateScoreDisplay(ResultScore);

	// --------------------------------
	// ランキング表示
	// --------------------------------
	DrawRanking();
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

void SaveScore::UpdateScoreDisplay(int score)
{
	std::string text = std::to_string(score);

	int digitIndex = scoreDigits.size() - text.length();

	for (size_t i = 0; i < scoreDigits.size(); i++)
	{
		if (i < digitIndex)
		{
			scoreDigits[i]->numU = 0;
			scoreDigits[i]->numV = 0;
		}
		else
		{
			int num = text[i - digitIndex] - '0';

			scoreDigits[i]->numU = num % 5;
			scoreDigits[i]->numV = num / 5;
		}
	}
}

// =======================================
// ランキング更新
// =======================================
void SaveScore::UpdateRanking(int newScore)
{
	// 新しいスコアを追加
	rankingScores.push_back(newScore);

	// 高い順にソート
	std::sort(rankingScores.begin(), rankingScores.end(), std::greater<int>());

	// 5位までに制限
	if (rankingScores.size() > 5)
	{
		rankingScores.resize(5);
	}
}

// =======================================
// ランキング表示
// =======================================
void SaveScore::DrawRanking()
{
	// -----------------------------
	// ランキング数字の位置
	// -----------------------------
	float startX = 180.0f;  // 右側へ移動
	// ランキングの1位の位置
	float startY = 44.0f;
	float lineSpacing = 34.0f; // 行間

	for (int i = 0; i < 5; i++)
	{
		if (i >= rankingScores.size())
		{
			continue;
		}

		int score = rankingScores[i];

		std::string text = std::to_string(score);

		float spacing = 32.0f;

		for (int j = 0; j < text.length(); j++)
		{
			int num = text[j] - '0';

			Object* digit = AddObject()
				->SetPos(startX + (j * spacing), startY - (i * lineSpacing), 0.0f)
				->SetSize(64.0f, 64.0f, 0.0f);

			digit->Init("asset/scoretext.png", 5, 2);
			digit->SetSpriteSheet(5, 2);

			digit->numU = num % 5;
			digit->numV = num / 5;

			digit->SetUI(true);
		}
	}
}