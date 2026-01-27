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

    m_player = std::make_unique<Player>();


	Object* player = AddObject();
	player->Init("asset/Texture/player_Stand.png");
	// スプライトシート設定
	// player.png は 4×4 = 16フレームの想定
	player->SetSpriteSheet(6, 6);
	player->SetPos(0.0f, 0.0f, 0.0f);
	player->SetSize(100.0f, 100.0f, 0.0f);
    player->SetCollisionRadius(50.0f);
    m_player->SetObject(player);

	// ENEMY OBJECT
	Object* nenemy = AddObject();
	nenemy->Init("asset/Texture/NormalEnemy.png");
	nenemy->SetPos(200.0f, 0.0f, 0.0f);
	nenemy->SetSize(100.0f, 100.0f, 0.0f);
    nenemy->SetCollisionRadius(50.0f);


}

void GamePlay::UpdateScene(float deltaTime)
{

	// Player が未生成なら何もしない
	if (!m_player) return;

	// 描画用 Object は Player が保持している（SetObjectで紐づけ済み）
	Object* playerObj = m_player->GetObject();
	Object* nenemy = (objects.size() > 1) ? objects[1].get() : nullptr;
	if (!playerObj || !nenemy) return;

	// いったん現在位置を保存
	const auto oldPos = playerObj->GetPos();

	// Player ロジック更新（入力→アニメ更新→移動まで）
	if (deltaTime > 0.1f)
		deltaTime = 0.1f;
	m_player->Update(deltaTime);

	// 移動後に衝突していたら、元の位置へ戻す（押し戻し）
	if (playerObj->CheckCollision(*nenemy))
	{
		playerObj->SetPos(oldPos.x, oldPos.y, oldPos.z);
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