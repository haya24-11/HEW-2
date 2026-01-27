#include "GamePlay.h"
#include"input.h"
#include <iostream>
#include <Windows.h>
GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "InitScene" << std::endl;

	// =========================
	// PLAYER OBJECT 初期化
	// =========================
	Object* player = AddObject();
	player->Init("asset/Texture/player_Stand.png");
	// スプライトシート設定
	// player.png は 4×4 = 16フレームの想定
	player->SetSpriteSheet(6, 6);
	player->SetPos(0.0f, 0.0f, 0.0f);
	player->SetSize(100.0f, 100.0f, 0.0f);
    player->SetCollisionRadius(50.0f);
	// ENEMY OBJECT
	Object* nenemy = AddObject();
	nenemy->Init("asset/Texture/NormalEnemy.png");
	nenemy->SetPos(200.0f, 0.0f, 0.0f);
	nenemy->SetSize(100.0f, 100.0f, 0.0f);
    nenemy->SetCollisionRadius(50.0f);


}

void GamePlay::UpdateScene(float deltaTime)
{
    Object* player = (objects.size() > 0) ? objects[0].get() : nullptr;
    Object* nenemy = (objects.size() > 1) ? objects[1].get() : nullptr;
    if (!player || !nenemy) return;

    // プレイヤー移動
    float moveSpeed = 1.0f;

    // 移動前の位置を保存（壁のように通さないため）
    const float oldX = player->GetPos().x;
    const float oldY = player->GetPos().y;
    const float oldZ = player->GetPos().z;

    float posX = oldX;
    float posY = oldY;
    float posZ = oldZ;

    float inputX = 0.0f;
    float inputY = 0.0f;


    // 入力取得（矢印キー / WASD）
    if (Input::GetKeyPress(VK_LEFT) || Input::GetKeyPress('A')) inputX -= 5.0f;
    if (Input::GetKeyPress(VK_RIGHT) || Input::GetKeyPress('D')) inputX += 5.0f;
    if (Input::GetKeyPress(VK_UP) || Input::GetKeyPress('W')) inputY += 5.0f;
    if (Input::GetKeyPress(VK_DOWN) || Input::GetKeyPress('S')) inputY -= 5.0f;

    if (inputX != 0 && inputY != 0)
    {
        inputX *= 0.7071f;
        inputY *= 0.7071f;
    }

    posX += inputX * moveSpeed;
    posY += inputY * moveSpeed;

    player->SetPos(posX, posY, posZ);

    // ✅ 移動後に衝突していたら、元の位置へ戻す（これ以上進めないようにする）
    bool isColliding = player->CheckCollision(*nenemy);
    if (isColliding)
    {
        player->SetPos(oldX, oldY, oldZ);
    }

    if (Input::GetKeyTrigger(VK_SPACE))
        SetNextScene(SceneType::Result);
}

void GamePlay::DrawScene()
{
	for (auto& obj : objects)
	{
		if (obj.get() == m_player->GetObject())
		{
			obj->Draw(m_player->GetAnimFrame());
		}
		else
		{
			obj->Draw();
		}
	}
}

void GamePlay::UninitScene()
{
	std::cout << "UninitScene" << std::endl;
}