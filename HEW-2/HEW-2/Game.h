#pragma once


#pragma once
#include <iostream>



enum SceneName
{
	TITLE,
	STAGE1,
	RESULT
};

class Game
{
private:
	

public:
	Game(); // コンストラクタ
	~Game(); // デストラクタ




	static void Init(); // 初期化
	static void Update(); // 更新
	static void Draw(); // 描画
	static void Uninit(); // 終了処理

	
};
