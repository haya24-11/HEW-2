#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>
#include "MathTypes.h"
#include "Object.h"

/*
    GameObject
    ==========
    ・「ゲーム世界に存在するもの」の最下層基底クラス
    ・位置
    ・当たり判定
    ・生存フラグ
    ・更新インターフェース
    だけを責務として持つ

    ※ 描画、入力、シーン遷移、AI などは一切持たない
*/

class GameObject : public Object
{
public:
    virtual ~GameObject() = default;

    // Scene から毎フレーム呼ばれる想定
    virtual void Update(float deltaTime) = 0;

    // 位置取得（読み取り専用）
    const DirectX::SimpleMath::Vector2& GetPosition()const;

    // 位置設定（Scene / Controller 側から操作）
    void SetPosition(const DirectX::SimpleMath::Vector2& pos);

    // 生存状態確認
    bool IsAlive()const;

protected:
    DirectX::SimpleMath::Vector2 position{ 0.0f,0.0f };
    float collisionRadius = 0.0f;   // 仮の当たり判定
    bool isAlive = true;
};