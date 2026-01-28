#pragma once
#include "Scene.h"
class Title :
    public Scene
{
public:
    Title();

    //テスト操作用
    int TitleMenu = 1;
    int m_count = 0;


    //タイトルUIオブジェクト一覧
    Object* TitleLogo; //タイトルロゴ
    Object* TitleBackground; //タイトル画面の背景
    Object* GameStartLogo; //ゲームスタート文字
    Object* ScoreLogo; //スコア文字
    Object* ExitLogo; //終了文字

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

