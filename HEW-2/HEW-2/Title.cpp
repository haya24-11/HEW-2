#include "Title.h"
#include "Game.h"

Object* obj = new Object;

Title::Title():Scene(SceneType::Title)
{
}

void Title::InitScene()
{
	std::cout << "a" << std::endl;
	
	Object* obj = AddObject();
	obj->Init("asset/titlerogo.png");
	obj->SetPos(0.0f, 0.0f, 0.0f);
	obj->SetSize(640.0f, 480.0f, 0.0f);
	obj->SetAngle(0.0f);

	// サウンドセット
	//Game::GetInstance()->GetSound().Play(SOUND_LABEL_BGM000);
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
	for (auto& obj : objects)
	{
		obj->Draw();
	}

}
void Title::UninitScene()
{

}