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
        cfg.stopDist = 100.0f;//敵が近づいたら止まる

        cfg.SetTexture("asset/Texture/player_attack_heavy.png");
        cfg.SetSpriteSheet(6, 5);
        cfg.SetAnim(0, 8, 0.12f, true); // 0~7, count=8


        return cfg;
    }
protected:
    void SetupAnimation() override;
    void ApplyWalkVisual() override;
    void Attack() override;
};

