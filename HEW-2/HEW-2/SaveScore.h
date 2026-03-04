#pragma once
#include "Scene.h"
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

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;
    void CreateNumberText(float startX, float y, std::string text);
};