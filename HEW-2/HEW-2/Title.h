#pragma once
#include "Scene.h"
#include <Xinput.h>

class Title : public Scene
{
public:
    Title();

    // テスト操作用
    int TitleMenu = 1;
    int m_count = 0;
    WORD m_prevButtons = 0;

    // タイトルUIオブジェクト一覧
    Object* TitleLogo = nullptr;
    Object* TitleBackground = nullptr;
    Object* title_backlogo = nullptr;
    Object* GameStartLogo = nullptr;
    Object* ScoreLogo = nullptr;
    Object* ExitLogo = nullptr;

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

    // =========================
    // タイトル画面：スライム歩行演出用
    // =========================
    struct WalkAnim
    {
        int start = 0;       // 開始フレーム（0）
        int count = 32;      // 8x4=32
        float sec = 0.10f;   // 1フレーム時間
        bool loop = true;
    };

    struct SlimeWalker
    {
        Object* obj = nullptr;
        WalkAnim anim;

        int frame = 0;
        float frameTimer = 0.0f;

        float speed = 25.0f;     // px/sec
        float dir = 1.0f;        // 右=+1 / 左=-1
        float y = 0.0f;

        float baseY = 0.0f;    
        float waveAmp = 12.0f;  
        float waveSpd = 1.5f;  
        float waveT = 0.0f;    

        float baseSpeed = 20.0f;     // 基準速度
        float spdAmp = 6.0f;         // 速度ゆらぎ幅
        float spdSpd = 1.2f;         // 速度ゆらぎ速度
        float spdT = 0.0f;           // 内部タイマー

        float xMin = -1200.0f;
        float xMax = 1200.0f;

        float respawnX = -1000.0f;


    };

    SlimeWalker m_green[3];
    SlimeWalker m_blue[3];
    SlimeWalker m_red[3];

private:
    static void UpdateSlimeWalker(SlimeWalker& s, float dt);
};