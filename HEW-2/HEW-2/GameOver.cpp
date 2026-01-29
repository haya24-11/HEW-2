#include "GameOver.h"

GameOver::GameOver() :Scene(SceneType::GameOver)
{
}

void GameOver::InitScene()
{
	std::cout << "o" << std::endl;

}

void GameOver::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE) || Input::GetButtonTrigger(XINPUT_RIGHT_SHOULDER))
	{
		SetNextScene(SceneType::Title);
	}
}

void GameOver::DrawScene()
{
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void GameOver::UninitScene()
{

}
