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
    scenes[static_cast<int>(SceneType::Play)] = std::make_unique<Play>();
    scenes[static_cast<int>(SceneType::Result)] = std::make_unique<Result>();
    scenes[static_cast<int>(SceneType::GameOver)] = std::make_unique<GameOver>();

    Input::Create();
    scenes[static_cast<int>(currentScene)]->CommonInit();
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
    Input::Release();
}
