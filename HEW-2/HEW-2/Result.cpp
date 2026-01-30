#include "Result.h"

Result::Result():Scene(SceneType::Result)
{
}

void Result::InitScene()
{
	std::cout << "(Debug) ResultScene!" << std::endl;

	// リザルトウィンドウ(画像は仮です...)
	ResultWindow = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(1200.0f, 600.0f, 0.0f)
		->SetAngle(0.0f);
	ResultWindow->Init("asset/resultwindow.png");

	ResultWindow->SetUI(true);
	/*// プレイヤーキャラクター
	PlayerCharacter = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	PlayerCharacter->Init("asset/playercharacter.png");

	// スコアテキスト
	ScoreText = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	ScoreText->Init("asset/scoretext.png");

	// 次のウィンドウ（Next画面）
	NextWindow = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	NextWindow->Init("asset/nextwindow.png");
	*/
}

void Result::UpdateScene(float deltaTime)
{
	if (Input::GetKeyTrigger(VK_SPACE) || Input::GetButtonTrigger(XINPUT_B))
	
		SetNextScene(SceneType::GameOver);
}

void Result::DrawScene()
{
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void Result::UninitScene()
{

}