#include "GamePlay.h"
#include"input.h"

GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "InitScene" << std::endl;

	// PLAYER OBJECT
	Object* player = AddObject();
	player->Init("asset/Texture/player.png");
	player->SetPos(0.0f, 0.0f, 0.0f);
	player->SetSize(100.0f, 100.0f, 0.0f);

	// ENEMY OBJECT
	Object* enemy = AddObject();
	enemy->Init("asset/Texture/NormalEnemy.png");
	enemy->SetPos(200.0f, 0.0f, 0.0f);
	enemy->SetSize(100.0f, 100.0f, 0.0f);
}
void GamePlay::UpdateScene(float deltaTime)
{
    std::cout << "deltaTime: " << deltaTime << std::endl;

    float moveSpeed = 1.0f; 
    Object* player = objects[0].get();


    float posX = player->GetPos().x;
    float posY = player->GetPos().y;
    float posZ = player->GetPos().z;

    float inputX = 0.0f;
    float inputY = 0.0f;

    if (Input::GetKeyPress(VK_LEFT) || Input::GetKeyPress('A'))  inputX -= 5.0f;
    if (Input::GetKeyPress(VK_RIGHT) || Input::GetKeyPress('D')) inputX += 5.0f;
    if (Input::GetKeyPress(VK_UP) || Input::GetKeyPress('W'))    inputY += 5.0f;
    if (Input::GetKeyPress(VK_DOWN) || Input::GetKeyPress('S'))  inputY -= 5.0f;

    if (inputX != 0 && inputY != 0)
    {
        inputX *= 0.7071f;
        inputY *= 0.7071f;
    }
    posX += inputX * moveSpeed;
    posY += inputY * moveSpeed;


    player->SetPos(posX, posY, posZ);

    if (Input::GetKeyTrigger(VK_SPACE))
        SetNextScene(SceneType::Result);
}



void GamePlay::DrawScene()
{
	for (auto& obj : objects)
	{
		obj->Draw();
	}
}
void GamePlay::UninitScene()
{
	std::cout << "UninitScene" << std::endl;
}