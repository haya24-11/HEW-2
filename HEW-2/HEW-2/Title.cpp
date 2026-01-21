#include "Title.h"

Title::Title():Scene(SceneType::Title)
{
}

void Title::InitScene()
{
	std::cout << "a" << std::endl;
}

void Title::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE))
	{
		SetNextScene(SceneType::GamePlay);
	}
}

void Title::DrawScene()
{

}
void Title::UninitScene()
{

}