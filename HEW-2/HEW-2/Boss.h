#pragma once
#include "Enemy.h"

// Boss
// ====
// ・Enemy を継承したボス敵
// ・スポーン設定やアニメ設定を上書きする
class Boss : public Enemy
{
public:
    Boss();

    // スポーン設定
    SpawnConfig GetSpawnConfig() const override
    {
        SpawnConfig cfg;

        cfg.texture = "asset/Texture/BossWalk.png";
        cfg.sizeX = 400.0f;
        cfg.sizeY = 400.0f;
        cfg.collisionRadius = 70.0f;

        cfg.interval = 9999.0f; // 再スポーンさせない
        cfg.maxAlive = 1;

        cfg.minDist = 300.0f;
        cfg.maxDist = 800.0f;

        cfg.stopDist = 200.0f;  // 近づいたら止まる距離（任意）
        cfg.hp = 2000;
        cfg.power = 10;

        // 死亡演出（必要なら）
        cfg.dieDelay = 1.0f;        // 死亡モーション表示時間
        cfg.disappearDelay = 0.0f;  // 追加で残す時間

        return cfg;
    }

protected:
    void SetupAnimation() override;
    void ApplyWalkVisual() override;
    void ApplyIdleVisual() override;
    void ApplyHitVisual() override;
    void ApplyDieVisual() override;

    void Attack() override;

    // ボス専用挙動（必要なら）
    void OnDamaged(int damage) override;
};
