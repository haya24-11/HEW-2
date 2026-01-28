#pragma once
#include <iostream>
#include "Title.h"
#include "GamePlay.h"
#include "Result.h"
#include "GameOver.h"
#include "input.h"
#include "Object.h"

class Game
{
private:

	std::unique_ptr<Scene> scenes[static_cast<int>(SceneType::GameOver) + 1];//シーンタイプの総数のサイズを確保
	static constexpr SceneType startupScene = SceneType::Title;
	SceneType currentScene = SceneType::Title;// 現在のシーン（タイトルで初期化）

public:
	Game(); // コンストラクタ
	~Game(); // デストラクタ

	void Init(); // 初期化
	void Update(float fps); // 更新
	void Draw(); // 描画
	void Uninit(); // 終了処理

	
};
