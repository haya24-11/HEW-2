#include "Result.h"

Result::Result() :Scene(SceneType::Result)
{



}

void Result::InitScene()
{

    PlayWallpaper = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(1670.0f, 940.0f, 0.0f);
    PlayWallpaper->Init("asset/wallpaper.png");
    PlayWallpaper->SetUI(true);

    ResultWindow = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(900.0f, 500.0f, 0.0f);
    ResultWindow->Init("asset/resultwindow.png");
    ResultWindow->SetUI(true);

    int kills = data.monsterKills;
    int combo = data.maxCombo;
    int seconds = (int)data.playTime;

    // スコア計算(仮)
    int totalScore = (kills * 4000) + (combo * 2000) + max(0, (600 - seconds) * 100);

    // 討伐数
    CreateNumberText(150.0f, 100.0f, std::to_string(kills));

    // 最大コンボ
    CreateNumberText(150.0f, 50.0f, std::to_string(combo));

    // 戦闘時間 (分：秒秒)
    char timeStr[16];
    sprintf_s(timeStr, "%d:%02d", seconds / 60, seconds % 60);
    CreateNumberText(150.0f, 0.0f, timeStr);

    // 合計スコア
    CreateNumberText(100.0f, -80.0f, std::to_string(totalScore));

    //文字
    ScoreText_Text = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(300.0f, 180.0f, 0.0f);
    ScoreText_Text->Init("asset/scoretext_moji.png");
    ScoreText_Text->SetUI(true);


    ScoreText_Score = AddObject()
        ->SetPos(-80.0f, -130.0f, 0.0f)
        ->SetSize(200.0f, 100.0f, 0.0f);
    ScoreText_Score->Init("asset/score_moji.png");
    ScoreText_Score->SetUI(true);

    //キャラクター
    PlayerCharacter = AddObject()
        ->SetPos(-270.0f, 0.0f, 0.0f)
        ->SetSize(220.0f, 250.0f, 0.0f);
    PlayerCharacter->Init("asset/playerwindow.png");
    PlayerCharacter->SetUI(true);

    //次へボタン
    NextWindow = AddObject()
        ->SetPos(300.0f, -200.0f, 0.0f)
        ->SetSize(200.0f, 50.0f, 0.0f);
    NextWindow->Init("asset/nextwindow.png");
    NextWindow->SetUI(true);
}

void Result::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE) || Input::GetButtonTrigger(XINPUT_B))

		SetNextScene(SceneType::SaveScore);
}

void Result::DrawScene()
{
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void Result::UninitScene()
{

}

void Result::CreateNumberText(float startX, float y, std::string text)
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

