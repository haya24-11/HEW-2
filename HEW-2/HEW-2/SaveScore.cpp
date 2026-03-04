#include "SaveScore.h"

SaveScore::SaveScore() :Scene(SceneType::SaveScore)
{
}

void SaveScore::InitScene()
{
	// PlayWallpaper
	PlayWallpaper = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1670.0f, 940.0f, 0.0f);
	PlayWallpaper->Init("asset/wallpaper.png");
	PlayWallpaper->SetUI(true);

	// ResultWindow
	ResultWindow = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(900.0f, 500.0f, 0.0f);
	ResultWindow->Init("asset/resultwindow.png");
	ResultWindow->SetUI(true);

	// TitleWindow
	TitleWindow = AddObject()
		->SetPos(0.0f, 200.0f, 0.0f)
		->SetSize(300.0f, 60.0f, 0.0f);
	TitleWindow->Init("asset/titlewindow.png");
	TitleWindow->SetUI(true);

	// PlayerCharacter
	PlayerCharacter = AddObject()
		->SetPos(-250.0f, 0.0f, 0.0f)
		->SetSize(400.0f, 400.0f, 0.0f);
	PlayerCharacter->Init("asset/playerwindow.png");
	PlayerCharacter->SetUI(true);

	// ScoreText_Text
	ScoreText_Text = AddObject()
		->SetPos(-50.0f, 100.0f, 0.0f)
		->SetSize(250.0f, 40.0f, 0.0f);
	ScoreText_Text->Init("asset/ranking.png");
	ScoreText_Text->SetUI(true);

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
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void SaveScore::UninitScene()
{

}

void SaveScore::CreateNumberText(float startX, float y, std::string text)
{
	float spacing = 32.0f; // 1文字進む幅（フォントサイズに合わせて調整）

	for (int i = 0; i < text.length(); ++i) {
		float nU = 0.0f;

		// 文字に応じた numU を設定
		if (text[i] >= '0' && text[i] <= '9') {
			nU = (float)(text[i] - '0');
		}
		else if (text[i] == ':') {
			nU = 10.0f; // 11番目のコロン
		}

		Object* obj = AddObject()
			->SetPos(startX + (i * spacing) + 40.0f, y + -50.0f, 0.0f)
			->SetSize(32.0f, 32.0f, 0.0f); // 表示サイズ

		obj->Init("asset/scoretext.png", 11, 1);
		obj->SetSpriteSheet(11, 1);
		obj->numU = nU;     // 1文字=1.0の設定を適用
		obj->SetUI(true);   // UIとして描画
	}
}