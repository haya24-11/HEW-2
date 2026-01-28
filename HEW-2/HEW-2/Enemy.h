#pragma once
#include "Chara.h"
#include "Object.h"

// Enemy
class Enemy : public Chara
{
public:
    Enemy();

    void Update(float deltaTime) override;
    void Attack() override;
    void TakeDamage(int damage);

    // target 설정(player)
    void SetTarget(Object* target) { m_target = target; }

    void SetChaseStopDistance(float d) { chaseStopDistance = d; }
    float GetChaseStopDistance() const { return chaseStopDistance; }

    void SetChaseEnabled(bool enabled) { chaseEnabled = enabled; }
    bool IsChaseEnabled() const { return chaseEnabled; }

    void KnockBack(const DirectX::SimpleMath::Vector2& force);

    // =========================
    // ✅ Random Spawn 設定
    // =========================
    struct SpawnConfig
    {
        float minDist = 200.0f;     // 플레이어부터 최소 거리
        float maxDist = 400.0f;     // 플레이어부터 최대 거리
        float interval = 2.0f;      // 스폰 간격(초)
        int   maxAlive = 10;        // 동시에 존재 가능한 최대 수
        float weight = 1.0f;        // 랜덤 선택 가중치(확률)

        // 생성될 Object 설정
        const char* texture = "asset/Texture/NormalEnemy.png";
        float sizeX = 100.0f;
        float sizeY = 100.0f;
        float collisionRadius = 50.0f;

        // (선택) 스폰 후 추적 멈춤 거리도 여기서 통일 가능
        float stopDist = 0.0f; // 0이면 끝까지 추적
    };

    // 각 적 타입이 자기 스폰 설정을 제공
    virtual SpawnConfig GetSpawnConfig() const { return SpawnConfig(); }

protected:
    bool isBoss = false;

    Object* m_target = nullptr;
    float chaseStopDistance = 0.0f;
    bool chaseEnabled = true;

    DirectX::SimpleMath::Vector2 knockBackVelocity{ 0.0f,0.0f };
    float knockBackTimer = 0.0f;
    float knockBackDuration = 0.15f;
};
