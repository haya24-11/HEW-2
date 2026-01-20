#pragma once
#include "GameObject.h"
#include "SceneTypes.h"

/*
    Chara
    =====
    ・キャラクター共通の状態を持つクラス
    ・HP
    ・基礎攻撃力（power）
    ・状態値（status）
    ・移動
    ・攻撃（抽象）

    ※ Exp / Skill / Mode / Combo には一切依存しない
*/

class Chara : public GameObject
{
public:
    virtual ~Chara() = default;

    // HP監視など最低限の更新
    void Update(float deltaTime)override;

    // 移動処理（速度や入力は外部で決める）
    virtual void Move(const DirectX::SimpleMath::Vector2& direction);

    // 攻撃はキャラ種別ごとに異なるため純粋仮想
    virtual void Attack() = 0;

    int GetHp() const;
    

protected:
    int hp = 1;
    int power = 1; // 基礎攻撃力（スキルで変化）
    int status = 0; // 状態管理用（仮）
};;