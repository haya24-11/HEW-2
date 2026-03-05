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

    Object* TitleBackLogo = nullptr;

    float m_logoTime = 0.0f;     // 揺れ時間
    float m_logoCenterX = 0.0f;  // 元のX位置
    float m_logoBreathTime = 0.0f;

    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;
    WORD m_prevButtons = 0;
};

