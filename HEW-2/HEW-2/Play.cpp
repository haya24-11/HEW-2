#include "Play.h"

Play::Play():Scene(SceneType::Play)
{
}

void Play::InitScene()
{
	std::cout << "i" << std::endl;
}

void Play::UpdateScene(float deltaTime)
{

	if (Input::GetKeyTrigger(VK_SPACE))
	{
		SetNextScene(SceneType::Result);
	}
}

void Play::DrawScene()
{

}
void Play::UninitScene()
{

}