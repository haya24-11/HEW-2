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
    scenes[static_cast<int>(SceneType::SaveScore)] = std::make_unique<SaveScore>();
    scenes[static_cast<int>(SceneType::GameOver)] = std::make_unique<GameOver>();

    Input::Create();
    scenes[static_cast<int>(currentScene)]->CommonInit();

    // SoundManager 初期化
    SoundManager::GetInstance().Init();
    Sound::GetInstance()->Init();

    // ★ 起動時BGMは即再生（フェードなし）
    SOUND_LABEL startBGM = GetBGMFromScene(currentScene);
    SoundManager::GetInstance().Play(startBGM);

    // SoundManager 側の状態も合わせる
    SoundManager::GetInstance().SetCurrentBGM(startBGM);

    ChangeBGM(currentScene);
}

//更新処理
void Game::Update(float fps)
{
    Input::Update();

    SoundManager::GetInstance().Update(1.0f / fps);

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
    Sound::GetInstance()->Uninit();
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
    SoundManager::GetInstance().RequestBGM(nextBGM);

    // 同じBGMなら何もしない
    if (nextBGM == SOUND_LABEL_MAX || currentBGM == nextBGM)
        return;

    auto& sound = SoundManager::GetInstance();
    SoundManager::GetInstance().PlayBGMFade(nextBGM, 1.0f);

    // 以前のBGMを止める
    if (currentBGM != SOUND_LABEL_MAX)
    {
        sound.Stop(currentBGM);
    }

    // フェードアウト → フェードイン
    sound.FadeOutBGM(0.5f);
    sound.FadeInBGM(nextBGM, 0.5f);

    // 新しいBGMを再生
    sound.Play(nextBGM);
    currentBGM = nextBGM;
}