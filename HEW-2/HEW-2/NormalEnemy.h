#pragma once
#include "Enemy.h"

class NormalEnemy :public Enemy
{
public:
	NormalEnemy();
    SpawnConfig GetSpawnConfig() const override
    {
        SpawnConfig cfg;
        cfg.interval = 2.0f;   // ✅ リスポーン（スポーン）間隔（秒）：0.7秒ごとに出現
        cfg.maxAlive = 10;     // （任意）同時に存在できる最大数
        cfg.minDist = 200.0f; // （任意）プレイヤーからの最小スポーン距離
        cfg.maxDist = 400.0f; // （任意）プレイヤーからの最大スポーン距離
        cfg.sizeX = 30.0f;
        cfg.sizeY = 30.0f;
        cfg.collisionRadius = 15.0f;
        return cfg;
    }

};

