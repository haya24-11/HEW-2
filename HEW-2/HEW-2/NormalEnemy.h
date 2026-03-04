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
        cfg.maxAlive = 20;     // （任意）同時に存在できる最大数
        cfg.minDist = 300.0f; // （任意）プレイヤーからの最小スポーン距離
        cfg.maxDist = 800.0f; // （任意）プレイヤーからの最大スポーン距離
        cfg.sizeX = 100.0f;
        cfg.sizeY = 100.0f;
        cfg.collisionRadius = 50.0f;
        cfg.stopDist = 200.0f;//敵が近づいたら止まる
        cfg.hp = 5;
        cfg.power = 3;
        return cfg;
    }

protected:
    void SetupAnimation() override;

    void ApplyWalkVisual() override;
    void ApplyIdleVisual() override;
    void ApplyHitVisual() override;
    void ApplyDieVisual() override;

    void OnDamaged(int damage) override;

    void Attack() override;
};

