#include "Title.h"
#include "Game.h"

Object* obj = new Object;

Title::Title():Scene(SceneType::Title)
{
}

void Title::InitScene()
{

	// �T�E���h�Z�b�g
	//Game::GetInstance()->GetSound().Play(SOUND_LABEL_BGM000);


	/*  ����
	* 
	* ��ʂ�ʂ�悤��
	* �^�C�g����ʂ̃X�R�A���
	* �Q�[���v���CUI�̂�
	* ���U���gUI
	* �Q�[���I�[�o�[UI�̂�
		�����F����(�������START,SCORE,EXIT�Z����)
		���˗��o���̂�
		�ꕶ����������(�\��,�܂���ł���)
		������v���k
	*/
	std::cout << "(Debug) TitleScene!" << std::endl;
	
	//�^�C�g���w�i
	TitleBackground = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f) //�ʒu
		->SetSize(1670.0f, 940.0f, 0.0f) //�傫��
		->SetAngle(0.0f); //�p�x
	TitleBackground->Init("asset/titlebackground.jpg"); //�\���摜

	//�^�C�g�����S
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
	//if (Input::GetKeyTrigger(VK_SPACE) || Input::GetButtonTrigger(XINPUT_RIGHT_SHOULDER))

	m_count++;

	// ���j���[����(��)
	TitleMenu += (Input::GetKeyTrigger(VK_DOWN) - Input::GetKeyTrigger(VK_UP));

	switch (TitleMenu) 
	{

	case 1: //GAMESTART��I�����Ă���ꍇ

		if (Input::GetKeyTrigger(VK_SPACE)) //���݂̓X�y�[�X�L�[�Ńv���C��ʂɔ�т܂�
		{
			SetNextScene(SceneType::GamePlay);
		}

		if (m_count >= 30) { 

			if (GameStartLogo->numU == 0) //�I�����W�̏ꍇ�̓C�G���[��
			{

				GameStartLogo->numU = 1;


			}
			else //�C�G���[�̏ꍇ�̓I�����W��
			{

				GameStartLogo->numU = 0;

			}

			m_count = 0;
		}
		ScoreLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 2: //SCORE��I�����Ă���ꍇ

		if (m_count >= 30) {

			if (ScoreLogo->numU == 0) //�I�����W�̏ꍇ�̓C�G���[��
			{

				ScoreLogo->numU = 1;


			}
			else //�C�G���[�̏ꍇ�̓I�����W��
			{

				ScoreLogo->numU = 0;

			}

			m_count = 0;
		}
		GameStartLogo->numU = 0;
		ExitLogo->numU = 0;
		break;

	case 3: //EXIT��I�����Ă���ꍇ

		if (m_count >= 30) {

			if (ExitLogo->numU == 0)//�I�����W�̏ꍇ�̓C�G���[��
			{

				ExitLogo->numU = 1;


			}
			else //�C�G���[�̏ꍇ�̓I�����W��
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