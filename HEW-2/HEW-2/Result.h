#pragma once
#include "Scene.h"
class Result :
    public Scene
{
public:
    Result();

    //リザルトUIオブジェクト一覧
    Object* ResultWindow; //リザルトのウィンドウ
    Object* PlayerCharacter; //キャラクター
    Object* ScoreText; //スコア表示のテキスト(仮。いらない場合は削除する予定。)
    Object* NextWindow; //次へのテキストがあるウィンドウ

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

