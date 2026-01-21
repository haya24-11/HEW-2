#include "Result.h"

Result::Result():Scene(SceneType::Result)
{
}

void Result::InitScene()
{
	std::cout << "u" << std::endl;
}

void Result::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE))
	{
		SetNextScene(SceneType::GameOver);
	}
}

void Result::DrawScene()
{

}
void Result::UninitScene()
{

}