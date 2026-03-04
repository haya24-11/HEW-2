// Chara.h
#pragma once
#include "GameObject.h"
#include "Object.h"
#include <SimpleMath.h>

class Chara : public GameObject
{
public:
    virtual ~Chara() = default;

    Object* GetObject() const { return m_object; }
    void SetObject(Object* obj) { m_object = obj; }

    // HP監視など最低限の更新
    void Update(float deltaTime) override;

    virtual void Move(const DirectX::SimpleMath::Vector2& direction, float deltaTime);
    virtual void Attack() = 0;

    int GetHp() const { return hp; }
    int GetPower() const { return power; }

    // ✅ ダメージ入口（Enemyがoverrideできる）
    virtual void TakeDamage(int dmg);

    float GetHeavyMul() const { return heavyDamageMul; }
    void SetHeavyMul(float m) { heavyDamageMul = m; }

protected:
    // ✅ HPが0になった瞬間の処理（派生で差し替え）
    virtual void OnHpZero();
    float heavyDamageMul = 2.0f; 
protected:
    Object* m_object = nullptr;
    DirectX::SimpleMath::Vector3 position{ 0,0,0 };

    int hp = 1;
    int power = 1;
    float moveSpeed = 300.0f;

};
