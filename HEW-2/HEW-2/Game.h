#pragma once
#include <iostream>
#include "Title.h"
#include "Play.h"
#include "Result.h"
#include "GameOver.h"

class Game
{
private:

	std::unique_ptr<Scene> scenes[static_cast<int>(SceneType::GameOver) + 1];//シーンタイプの総数のサイズを確保
	static constexpr SceneType startupScene = SceneType::Title;
	SceneType currentScene = SceneType::Title;// 現在のシーン（タイトルで初期化）

public:
	Game(); // コンストラクタ
	~Game(); // デストラクタ

	static void Init(); // 初期化
	static void Update(); // 更新
	static void Draw(); // 描画
	static void Uninit(); // 終了処理

	
};
