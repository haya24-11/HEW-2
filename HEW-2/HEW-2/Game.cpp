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
}

//更新処理
void Game::Update()
{
}

//描画処理
void Game::Draw()
{
    // ウィンドウのDC取得
    HDC hdc = GetDC(Application::GetWindow());

    // 背景を塗る（青）
    RECT rc;
    GetClientRect(Application::GetWindow(), &rc);
    HBRUSH brush = CreateSolidBrush(RGB(100, 180, 255));
    FillRect(hdc, &rc, brush);

    // 後始末
    DeleteObject(brush);
    ReleaseDC(Application::GetWindow(), hdc);
}


//終了処理
void Game::Uninit()
{
}
