#pragma once
#include "Object.h"

/*
    GameObject
    ==========
    ・ゲーム世界に存在するものの基底
    ・生存フラグ
    ・更新インターフェース
*/
class GameObject : public Object
{
public:
    virtual ~GameObject() = default;

    // Scene から毎フレーム呼ばれる想定
    virtual void Update(float deltaTime) = 0;

    // 生存状態
    bool IsAlive() const { return isAlive; }

    // 生存状態を設定
    void SetAlive(bool alive) { isAlive = alive; }


protected:

    DirectX::SimpleMath::Vector2 position{ 0.0f, 0.0f };
    bool isAlive = true;
};
