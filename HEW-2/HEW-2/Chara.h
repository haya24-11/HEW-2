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

    void SetObject(Object* obj) { m_object = obj; }
    /*
        Move
        ----
        ・direction : 移動方向ベクトル（正規化されていなくてよい）
        ・deltaTime : フレーム依存を防ぐための経過時間
    */
    // 移動処理（速度や入力は外部で決める）
    virtual void Move(const DirectX::SimpleMath::Vector2& direction,float deltaTime);

    // 攻撃はキャラ種別ごとに異なるため純粋仮想
    virtual void Attack() = 0;

    int GetHp() const;
    

protected:
    Object* m_object = nullptr;

    DirectX::SimpleMath::Vector3 position{};
    int hp = 1;
    int power = 1; // 基礎攻撃力（スキルで変化）
    int status = 0; // 状態管理用（仮）
 
    // １秒当たりの移動量
    float moveSpeed = 300.0f;
};