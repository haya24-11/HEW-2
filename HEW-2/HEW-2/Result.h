#pragma once
#include "Scene.h"
#include "EnemySpawner.h"
#include "ComboManager.h"
class Result :
    public Scene
{
public:
    Result();

    //リザルトUIオブジェクト一覧
    Object* ResultWindow; //リザルトのウィンドウ
    Object* PlayWallpaper; //リザルトのウィンドウ
    Object* PlayerCharacter; //キャラクター
    Object* ScoreText_Text;   //スコア表示のテキスト(文字)
    Object* ScoreText_Score;   //スコア表示のテキスト(スコアのみ文字)
    Object* ScoreText_Coron; //スコア表示のテキスト(数字)　
    Object* NextWindow; //次へのテキストがあるウィンドウ

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;
    void CreateNumberText(float startX, float y, std::string text);


};

