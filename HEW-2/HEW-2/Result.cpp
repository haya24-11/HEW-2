#define NOMINMAX
#include "Result.h"
#include <algorithm>
#include <cstdio>

Result::Result() :Scene(SceneType::Result)
{



}

void Result::InitScene()
{
    // 背景
    PlayWallpaper = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(1670.0f, 940.0f, 0.0f);
    PlayWallpaper->Init("asset/wallpaper.png");
    PlayWallpaper->SetUI(true);

    // 結果ウィンドウ
    ResultWindow = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(900.0f, 500.0f, 0.0f);
    ResultWindow->Init("asset/resultwindow.png");
    ResultWindow->SetUI(true);

    resultData = Scene::GetResultData();

    int Kills = resultData.monsterKills;
    int Combo = resultData.maxCombo;
    int Seconds = (int)resultData.playTime;
    bool Clear = resultData.isClear;

    //----------------------------------
    // スコア計算
    //----------------------------------

    int killScore = Kills * 4000;
    int comboScore = Combo * 2000;

    // 生存時間ボーナス
    int timeScore = (600 - Seconds) * 100;
    if (timeScore < 0) timeScore = 0;

    // クリアボーナス
    int clearScore = Clear ? 30000 : 0;

    totalScore =
        killScore +
        comboScore +
        timeScore +
        clearScore;

    //----------------------------------
    // 討伐数
    //----------------------------------

    CreateNumberText(150.0f, 100.0f, std::to_string(Kills));

    //----------------------------------
    // 最大コンボ
    //----------------------------------

    CreateNumberText(150.0f, 50.0f, std::to_string(Combo));

    //----------------------------------
    // 生存時間
    //----------------------------------

    int Mins = Seconds / 60;
    int Secs = Seconds % 60;

    CreateNumberText(150.0f, 0.0f, std::to_string(Mins));

    // コロン
    Object* colon = AddObject()
        ->SetPos(205.0f, -50.0f, 0.0f)
        ->SetSize(40.0f, 40.0f, 0.0f);

    colon->Init("asset/colon.png");
    colon->SetUI(true);

    char SecBuf[3];
    sprintf_s(SecBuf, "%02d", Secs);

    CreateNumberText(210.0f, 0.0f, SecBuf);

    //----------------------------------
    // 合計スコア
    //----------------------------------

    CreateNumberText(100.0f, -80.0f, std::to_string(totalScore));

    //----------------------------------
    // UI文字
    //----------------------------------

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
    ScoreText_Coron->Init("asset/colon.png");
    ScoreText_Coron->SetUI(true);

    //----------------------------------
    // キャラクター表示
    //----------------------------------

    PlayerCharacter = AddObject()
        ->SetPos(-270.0f, 0.0f, 0.0f)
        ->SetSize(220.0f, 250.0f, 0.0f);

    if (Clear)
        PlayerCharacter->Init("asset/clearplayer.png", 8, 3);
    else
        PlayerCharacter->Init("asset/overplayer.png", 8, 3);

    PlayerCharacter->SetSpriteSheet(8, 3);
    PlayerCharacter->SetUI(true);

    //----------------------------------
    // 次へ
    //----------------------------------

    NextWindow = AddObject()
        ->SetPos(300.0f, -200.0f, 0.0f)
        ->SetSize(200.0f, 50.0f, 0.0f);
    NextWindow->Init("asset/nextwindow.png");
    NextWindow->SetUI(true);

    //----------------------------------
    // デバッグ
    //----------------------------------

    std::cout << "Kills  : " << Kills << std::endl;
    std::cout << "Combo  : " << Combo << std::endl;
    std::cout << "Time   : " << Seconds << std::endl;
    std::cout << "Score  : " << totalScore << std::endl;
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

    for (size_t i = 0; i < text.length(); ++i) {

        // 文字に応じた numU を設定
        if (text[i] < '0' || text[i] > '9')
            continue;

        float nU = (float)(text[i] - '0');

        Object* obj = AddObject()
            ->SetPos(startX + (i * spacing) + 40.0f, y + -50.0f, 0.0f)
            ->SetSize(64.0f, 64.0f, 0.0f); // 表示サイズ

        obj->Init("asset/scoretext.png", 5, 2);
        obj->SetSpriteSheet(5, 2);
        obj->numU = nU;     // 1文字=1.0の設定を適用
        obj->SetUI(true);   // UIとして描画
    }
}

