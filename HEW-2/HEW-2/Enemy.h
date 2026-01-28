#pragma once
#include "Chara.h"

/*
    Enemy
    =====
    �E�G�L�������ʏ���
    �E��_���[�W
    �E�m�b�N�o�b�N
*/
﻿#include "Chara.h"
#include "Object.h"

// Enemy
// ============================================================
// ・敵キャラ共通クラス
// ・プレイヤー追跡、攻撃、被ダメージ処理などの基底
// ・スポーン設定(SpawnConfig)を各派生クラスが提供できる
// ============================================================
class Enemy : public Chara
{
public:
    Enemy();

    // 毎フレーム更新（追跡・移動・ノックバック処理など）
    void Update(float deltaTime) override;

    // 攻撃処理（派生で具体実装）
    void Attack() override;

    // ダメージを受ける
    void TakeDamage(int damage);

    // =========================
    // 追跡ターゲット設定（通常はプレイヤーObject）
    // =========================
    void SetTarget(Object* target) { m_target = target; }

    // 追跡停止距離（この距離以内なら止まる/減速などに使う想定）
    void SetChaseStopDistance(float d) { chaseStopDistance = d; }
    float GetChaseStopDistance() const { return chaseStopDistance; }

    // 追跡のON/OFF
    void SetChaseEnabled(bool enabled) { chaseEnabled = enabled; }
    bool IsChaseEnabled() const { return chaseEnabled; }

    // =========================
    // ノックバック
    // force : ノックバック方向×強さ（速度として扱う想定）
    // =========================
    void KnockBack(const DirectX::SimpleMath::Vector2& force);

    // =========================
    // ランダムスポーン設定
    // ・EnemySpawner が参照してスポーン位置/間隔/最大数などを制御
    // =========================
    struct SpawnConfig
    {
        float minDist = 200.0f;   // プレイヤーからの最小スポーン距離
        float maxDist = 400.0f;   // プレイヤーからの最大スポーン距離
        float interval = 2.0f;    // スポーン間隔（秒）
        int   maxAlive = 10;      // 同時に存在可能な最大数
        float weight = 1.0f;      // ランダム抽選の重み（確率）

        // 生成される Object の見た目/当たり判定
        const char* texture = "asset/Texture/NormalEnemy.png"; // 使用テクスチャ
        float sizeX = 100.0f;        // サイズX
        float sizeY = 100.0f;        // サイズY
        float collisionRadius = 50.0f; // 当たり判定半径

        // （任意）スポーン後の追跡停止距離
        // 0 の場合は最後まで追跡
        float stopDist = 0.0f;
    };

    // 各敵タイプが自分のスポーン設定を返す（派生クラスで override 推奨）
    virtual SpawnConfig GetSpawnConfig() const { return SpawnConfig(); }

protected:
    bool isBoss = false;     // ボス判定（必要なら派生で利用）

private:
    // �m�b�N�o�b�N�p
    DirectX::SimpleMath::Vector2 knockBackVelocity{ 0.0f,0.0f };
    float knockBackTimer = 0.0f;
    // 追跡ターゲット（プレイヤーなど）
    Object* m_target = nullptr;

    // 追跡停止距離（0なら停止しない想定）
    float chaseStopDistance = 0.0f;

    // 追跡有効フラグ
    bool chaseEnabled = true;

    // =========================
    // ノックバック用パラメータ
    // =========================
    DirectX::SimpleMath::Vector2 knockBackVelocity{ 0.0f, 0.0f }; // ノックバック速度
    float knockBackTimer = 0.0f;        // ノックバック経過時間
    float knockBackDuration = 0.15f;    // ノックバック継続時間
};
