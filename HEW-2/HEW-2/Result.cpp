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

    int Kills = data.monsterKills;
    int Combo = data.maxCombo;
    int Seconds = (int)data.playTime;
    bool Clear = data.isClear;
    int ResultScore = data.score;

    // スコア計算(仮)
    int TotalScore = (Kills * 4000) + (Combo * 2000) + max(0, (600 - Seconds) * 100);

    ResultScore = TotalScore;

    // 討伐数
    CreateNumberText(150.0f, 100.0f, std::to_string(Kills));

    // 最大コンボ
    CreateNumberText(150.0f, 50.0f, std::to_string(Combo));

    // 戦闘時間 (分：秒秒)
// 1. データの計算
    int TotalSeconds = (int)data.playTime;
    int Mins = TotalSeconds / 60; // 分
    int Secs = TotalSeconds % 60; // 秒

    //「分」の表示 
    CreateNumberText(150.0f, 0.0f, std::to_string(Mins));

    //「秒」の表示
    char SecBuf[3];
    sprintf_s(SecBuf, "%02d", Secs); // 9秒なら "09" になる
    CreateNumberText(210.0f, 0.0f, SecBuf);

    // 合計スコア
    CreateNumberText(100.0f, -80.0f, std::to_string(TotalScore));

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

    ScoreText_Coron = AddObject()
        ->SetPos(219.0f, -51.0f, 0.0f)
        ->SetSize(80.0f, 80.0f, 0.0f);
    ScoreText_Coron->Init("asset/coron.png");
    ScoreText_Coron->SetUI(true);
    

    //キャラクター
    PlayerCharacter = AddObject()
        ->SetPos(-270.0f, 0.0f, 0.0f)
        ->SetSize(220.0f, 250.0f, 0.0f);
    if (Clear == true) {
        PlayerCharacter->Init("asset/clearplayer.png", 8, 3);
    }
    else
    {
        PlayerCharacter->Init("asset/overplayer.png", 8, 3);
    }
    PlayerCharacter->SetSpriteSheet(8, 3);
    PlayerCharacter->SetUI(true);

    //次へボタン　
    NextWindow = AddObject()
        ->SetPos(300.0f, -200.0f, 0.0f)
        ->SetSize(200.0f, 50.0f, 0.0f);
    NextWindow->Init("asset/nextwindow.png");
    NextWindow->SetUI(true);

    std::cout << "(Debug) ResultScene!" << std::endl;
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

        Object* obj = AddObject()
            ->SetPos(startX + (i * spacing) + 40.0f, y + -50.0f, 0.0f)
            ->SetSize(64.0f, 64.0f, 0.0f); // 表示サイズ

        obj->Init("asset/scoretext.png", 5, 2);
        obj->SetSpriteSheet(5, 2);
        obj->numU = nU;     // 1文字=1.0の設定を適用
        obj->SetUI(true);   // UIとして描画
    }
}

