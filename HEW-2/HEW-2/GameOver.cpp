#include "GameOver.h"

GameOver::GameOver() :Scene(SceneType::GameOver)
{
}

void GameOver::InitScene()
{


	// 暗転
	BlackScreen = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1670.0f, 940.0f, 0.0f)
		->SetAngle(0.0f);
	BlackScreen->Init("asset/blackscreen.png");
	BlackScreen->SetColor(1.0f, 1.0f, 1.0f, 0.5f);

	// ゲームオーバーロゴ(作ってないのでテストでスコアオレンジ入れてます　差し替え待ちです)
	GameOverLogo = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(200.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	GameOverLogo->Init("asset/gameoverlogo.png");


	std::cout << "(Debug) GameOverScene!" << std::endl;
}

void GameOver::UpdateScene(float deltaTime)
{


	if (Input::GetKeyTrigger(VK_SPACE))
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
