#include "GameOver.h"

GameOver::GameOver() :Scene(SceneType::GameOver)
{
}

void GameOver::InitScene()
{
	std::cout << "o" << std::endl;



	// �Ó]
	BlackScreen = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1670.0f, 940.0f, 0.0f)
		->SetAngle(0.0f);
	BlackScreen->Init("asset/blackscreen.png");
	BlackScreen->SetColor(1.0f, 1.0f, 1.0f, 0.5f);

	// �Q�[���I�[�o�[���S(����ĂȂ��̂Ńe�X�g�ŃX�R�A�I�����W����Ă܂��@�����ւ��҂��ł�)
	GameOverLogo = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(200.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	GameOverLogo->Init("asset/gameoverlogo.png");


	std::cout << "(Debug) GameOverScene!" << std::endl;
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
