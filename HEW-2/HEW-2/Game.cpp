#include "Game.h"
#include "Application.h"
#include <windows.h>

//コンストラクタ
Game::Game()
{
}

//デストラクタ
Game::~Game()
{

}

//初期化処理
void Game::Init()
{
    scenes[static_cast<int>(SceneType::Title)] = std::make_unique<Title>();
    scenes[static_cast<int>(SceneType::GamePlay)] = std::make_unique<GamePlay>();
    scenes[static_cast<int>(SceneType::Result)] = std::make_unique<Result>();
    scenes[static_cast<int>(SceneType::GameOver)] = std::make_unique<GameOver>();

    Input::Create();
    scenes[static_cast<int>(currentScene)]->CommonInit();

    // SoundManager 初期化
    SoundManager::GetInstance().Init();

    ChangeBGM(currentScene);
}

//更新処理
void Game::Update(float fps)
{
    Input::Update();
    scenes[static_cast<int>(currentScene)]->UpdateScene(fps);

    SceneType nextScene = scenes[static_cast<int>(currentScene)]->GetNextScene();
    if (nextScene != SceneType::NONE)
    {
        // 今のシーンを終了して
        scenes[static_cast<int>(currentScene)]->ClearObject();

        // 次のシーンの初期化処理を行う
        scenes[static_cast<int>(nextScene)]->CommonInit();

        // 現在のシーン情報を更新
        currentScene = nextScene;

        // ★ ここでBGM切り替え
        ChangeBGM(currentScene);
    }
}

//描画処理
void Game::Draw()
{
    RendererDrawStart();

    scenes[static_cast<int>(currentScene)]->DrawScene();

    RendererDrawEnd();


}


//終了処理
void Game::Uninit()
{
    SoundManager::GetInstance().Uninit();
    Input::Release();
}

SOUND_LABEL Game::GetBGMFromScene(SceneType scene)
{
    switch (scene)
    {
    case SceneType::Title:
        return SOUND_LABEL_BGM_TITLE;

    case SceneType::GamePlay:
        return SOUND_LABEL_BGM_GAME;

    case SceneType::Result:
        return SOUND_LABEL_BGM_RESULT;

    case SceneType::GameOver:
        return SOUND_LABEL_BGM_GAMEOVER;

    default:
        return SOUND_LABEL_MAX;
    }
}

void Game::ChangeBGM(SceneType scene)
{
    SOUND_LABEL nextBGM = GetBGMFromScene(scene);

    // 同じBGMなら何もしない
    if (nextBGM == SOUND_LABEL_MAX || currentBGM == nextBGM)
        return;

    auto& sound = SoundManager::GetInstance();

    // 以前のBGMを止める
    if (currentBGM != SOUND_LABEL_MAX)
    {
        sound.Stop(currentBGM);
    }

    // 新しいBGMを再生
    sound.Play(nextBGM);
    currentBGM = nextBGM;
}