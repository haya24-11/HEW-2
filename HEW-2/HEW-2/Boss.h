#pragma once
#include "Enemy.h"

class Boss :public Enemy
{
public:
	
    Boss();

    // ボス専用スポーン設定
    SpawnConfig GetSpawnConfig() const override;

protected:
    void SetupAnimation() override;
    void ApplyWalkVisual() override;
    void ApplyIdleVisual() override;
    void ApplyHitVisual() override;
    void ApplyDieVisual() override;

    void Attack() override;

    // ボス専用の挙動（必要なら）
    void OnDamaged(int damage) override;
    };



