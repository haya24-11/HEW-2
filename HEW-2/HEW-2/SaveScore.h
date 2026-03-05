#pragma once
#include "Scene.h"
#include "ResultData.h"
class SaveScore :
    public Scene
{
public:
    SaveScore();

    Object* ResultWindow; //リザルトのウィンドウ　
    Object* PlayWallpaper; //リザルトのウィンドウ
    Object* TitleWindow; //TITLEのテキストがあるウィンドウ
    Object* PlayerCharacter; //キャラクター
    Object* ScoreText_Text;   //スコア表示のテキスト(文字)
    // =======================================
    // スコア数字UI
    // =======================================
    std::vector<Object*> scoreDigits;
    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;
    void UpdateScoreDisplay(int score);
};