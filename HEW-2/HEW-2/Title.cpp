#include "Title.h"
Object* obj = new Object;
Title::Title():Scene(SceneType::Title)
{
}

void Title::InitScene()
{



	/*  メモ
	* 
	* 一通り通るように
	* タイトル画面のスコア画面
	* ゲームプレイUIのみ
	* リザルトUI
	* ゲームオーバーUIのみ
		文字色調整(ちょっとSTART,SCORE,EXIT濃いめ)
		↑依頼出しのみ
		一文字ずつ動かす(予定,まだ先でおｋ)
		↑現状要相談
	*/
	std::cout << "(Debug) TitleScene!" << std::endl;
	
	//タイトル背景
	TitleBackground = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f) //位置
		->SetSize(1670.0f, 940.0f, 0.0f) //大きさ
		->SetAngle(0.0f); //角度
	TitleBackground->Init("asset/titlebackground.jpg"); //表示画像

	//タイトルロゴ
	TitleLogo = AddObject()
		->SetPos(-30.0f, 200.0f, 0.0f)
		->SetSize(550.0f, 450.0f, 0.0f)
		->SetAngle(0.0f);
	TitleLogo->Init("asset/titlelogo.png");

	//SCORE
	ScoreLogo = AddObject()
		->SetPos(20.0f, -250.0f, 0.0f)
		->SetSize(250.0f, 110.0f, 0.0f)
		->SetAngle(0.0f);
	ScoreLogo->Init("asset/score.png", 2, 1);
	
	//GAME START
	GameStartLogo = AddObject()
		->SetPos(20.0f, -100.0f, 0.0f)
		->SetSize(370.0f, 110.0f, 0.0f)
		->SetAngle(0.0f);
	GameStartLogo->Init("asset/gamestart.png", 2, 1);

	//EXIT
	ExitLogo = AddObject()
		->SetPos(720.0f, -400.0f, 0.0f)
		->SetSize(200.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	ExitLogo->Init("asset/exit.png", 2, 1);
	
}

void Title::UpdateScene(float deltaTime)
{

	m_count++;

	// メニュー操作(仮)
	TitleMenu += (Input::GetKeyTrigger(VK_DOWN) - Input::GetKeyTrigger(VK_UP));

	switch (TitleMenu) 
	{

	case 1: //GAMESTARTを選択している場合

		if (Input::GetKeyTrigger(VK_SPACE)) //現在はスペースキーでプレイ画面に飛びます
		{
			SetNextScene(SceneType::GamePlay);
		}

		if (m_count >= 30) { 

			if (GameStartLogo->numU == 0) //オレンジの場合はイエローに
			{

				GameStartLogo->numU = 1;


			}
			else //イエローの場合はオレンジに
			{

				GameStartLogo->numU = 0;

			}

			m_count = 0;
		}
		ScoreLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 2: //SCOREを選択している場合

		if (m_count >= 30) {

			if (ScoreLogo->numU == 0) //オレンジの場合はイエローに
			{

				ScoreLogo->numU = 1;


			}
			else //イエローの場合はオレンジに
			{

				ScoreLogo->numU = 0;

			}

			m_count = 0;
		}
		GameStartLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 3: //EXITを選択している場合

		if (m_count >= 30) {

			if (ExitLogo->numU == 0)//オレンジの場合はイエローに
			{

				ExitLogo->numU = 1;


			}
			else //イエローの場合はオレンジに
			{

				ExitLogo->numU = 0;

			}

			m_count = 0;
		}
		GameStartLogo->numU = 0;
		ScoreLogo->numU = 0;
		break;

	}


	if (1 > TitleMenu)
	{

		TitleMenu = 1;

	}
	else if (TitleMenu > 3)
	{

		TitleMenu = 3;

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