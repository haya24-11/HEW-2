#pragma once
#include "Scene.h"
class GameOver :
    public Scene
{
public:
    GameOver();

    //ゲームオーバーUIオブジェクト一覧
    Object* GameOverLogo; //ゲームオーバー文字
    Object* BlackScreen; //画面暗くする用？

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

