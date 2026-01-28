#include "GamePlay.h"

GamePlay::GamePlay():Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
	std::cout << "(Debug) GamePlayScene!" << std::endl;


	// 弱攻撃ボタン
	LightAttackButton = AddObject()
		->SetPos(300.0f, -250.0f, 0.0f)
		->SetSize(45.0f, 35.0f, 0.0f)
		->SetAngle(0.0f);
	LightAttackButton->Init("asset/lightattackbutton.png");

	// 強攻撃ボタン
	HeavyAttackButton = AddObject()
		->SetPos(260.0f, -220.0f, 0.0f)
		->SetSize(45.0f, 35.0f, 0.0f)
		->SetAngle(0.0f);
	HeavyAttackButton->Init("asset/heavyattackbutton.png");

	// プレイヤーアイコン
	PlayerIcon = AddObject()
		->SetPos(-745.0f, 385.0f, 0.0f)
		->SetSize(100.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	PlayerIcon->Init("asset/playericon.png");

	// プレイヤーHPバー
	PlayerHeartPointBar = AddObject()
		->SetPos(-580.0f, 390.0f, 0.0f)
		->SetSize(500.0f, 150.0f, 0.0f)
		->SetAngle(0.0f);
	PlayerHeartPointBar->Init("asset/playerheartpointbar.png");

	// バフアイコン
	for (int i = 0; i < 5; i++) {
		// オブジェクトを追加
		Object* newBuff = AddObject()
			->SetPos(-630.0f + (i * 50.0f), 300.0f, 0.0f) // 横に50ずつずらして配置する例
			->SetSize(50.0f, 50.0f, 1.0f)
			->SetAngle(0.0f);

		// 画像の初期化
		newBuff->Init("asset/bufficon.png");

		// vectorに追加して保持しておく
		BuffIcons.push_back(newBuff);
	}

	// 魔法陣
	MagicCircle = AddObject()
		->SetPos(800.0f, 450.0f, 0.0f)
		->SetSize(400.0f, 400.0f, 0.0f)
		->SetAngle(0.0f);
	MagicCircle->Init("asset/magiccircle.png");

	/* 敵HPバー
	EnemyHeartPointBar = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	EnemyHeartPointBar->Init("asset/enemyheartpointbar.png");
	*/
	/*
	// 経験値バー
	ExpBar = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	ExpBar->Init("asset/expbar.png");
	*/
	/*
	// コンボ表示
	Combo = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Combo->Init("asset/combo.png");
	*/
	/*
	// プレイヤー
	Player = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Player->Init("asset/player.png");
	*/
	/*
	// 敵
	Enemy = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Enemy->Init("asset/enemy.png");
	*/
}

void GamePlay::UpdateScene(float deltaTime)
{

	if (Input::GetKeyTrigger(VK_SPACE))
	{
		SetNextScene(SceneType::Result);
	}
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

}