#include "GamePlay.h"
#include"input.h"

GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "InitScene" << std::endl;

	// ==================================================
	// PLAYER OBJECT 初期化
	// ==================================================
	// ----- Idle（待機） Object -----
	Object* playerObj = AddObject();
	playerObj->Init("asset/Texture/player_idle.png");
	playerObj->SetSpriteSheet(6, 6);
	playerObj->SetSize(150.0f, 170.0f, 0.0f);
	playerObj->SetPos(0.0f, 0.0f, 0.0f);

	m_player = std::make_unique<Player>();
	m_player->SetObject(playerObj);

	// ==================================================
	// ENEMY OBJECT 初期化
	// ==================================================
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