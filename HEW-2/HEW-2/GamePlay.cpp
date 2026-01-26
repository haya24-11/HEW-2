#include "GamePlay.h"
#include"input.h"

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
	player->SetSize(150.0f, 170.0f, 0.0f);

    m_player = std::make_unique<Player>();
    m_player->SetObject(player);

	// =========================
	// ENEMY OBJECT 初期化
	// =========================
	Object* enemy = AddObject();
	enemy->Init("asset/Texture/NormalEnemy.png");
	enemy->SetPos(200.0f, 0.0f, 0.0f);
	enemy->SetSize(100.0f, 100.0f, 0.0f);
}

void GamePlay::UpdateScene(float deltaTime)
{
	if (deltaTime > 0.1f)
		deltaTime = 0.1f;

	m_player->Update(deltaTime);

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