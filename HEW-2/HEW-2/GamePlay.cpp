#include "GamePlay.h"
#include <iostream>
GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "[GamePlay]InitScene" << std::endl;

	// PLAYER OBJECT
	Object* player = AddObject();
	player->Init("asset/Texture/Player.png");
	player->SetPos(0.0f, 0.0f, 0.0f);
	player->SetSize(100.0f, 100.0f, 0.0f);

	// ENEMY OBJECT
	Object* enemy = AddObject();
	enemy->Init("asset/Texture/Enemy.png");
	enemy->SetPos(200.0f, 0.0f, 0.0f);
	enemy->SetSize(100.0f, 100.0f, 0.0f);
}

void GamePlay::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE))
	{
		SetNextScene(SceneType::Result);
	}
}

void GamePlay::DrawScene()
{
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void GamePlay::UninitScene()
{
	std::cout << "[GamePlay]UninitScene" << std::endl;
}

