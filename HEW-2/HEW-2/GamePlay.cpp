#include "GamePlay.h"

GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "i" << std::endl;
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

}