#pragma once
#include "Chara.h"
#include "Object.h"
#include "Animator.h"

class Enemy : public Chara
{
public:
    Enemy();

    // 毎フレーム更新（追跡・移動・ノックバック処理など）
    void Update(float deltaTime) override;

    // ダメージを受ける（被弾処理の入口）
    void TakeDamage(int damage);

    // 追跡ターゲット設定（通常はプレイヤーObject）
    void SetTarget(Object* target) { m_target = target; }

    // 追跡停止距離（この距離以内なら止まる/減速などに使う想定）
    void SetChaseStopDistance(float d) { chaseStopDistance = d; }
    float GetChaseStopDistance() const { return chaseStopDistance; }

    // 追跡のON/OFF
    void SetChaseEnabled(bool enabled) { chaseEnabled = enabled; }
    bool IsChaseEnabled() const { return chaseEnabled; }

    // ノックバック（force : 方向 × 強さ）
    void KnockBack(const DirectX::SimpleMath::Vector2& force);

    bool IsKnockBacking() const { return knockBackTimer > 0.0f; }

    // ノックバック速度を取得（敵同士の反動計算に使う）
    DirectX::SimpleMath::Vector2 GetKnockBackVelocity() const { return knockBackVelocity; }

    // ノックバックを強制停止（飛んでる敵を止める）
    void StopKnockBack()
    {
        knockBackTimer = 0.0f;
        knockBackVelocity = { 0.0f, 0.0f };
    }


    //  ノックバックは維持したまま、速度だけを差し替える（タイマーは維持）
    void SetKnockBackVelocity(const DirectX::SimpleMath::Vector2& v)
    {
        knockBackVelocity = v;
        if (knockBackTimer <= 0.0f)
            knockBackTimer = knockBackDuration;
    }

    // ============================================================
    // SpawnConfig
    // ・EnemySpawner が参照してスポーン位置/間隔/最大数などを制御
    // ・見た目（テクスチャ/スプライトシート）や当たり判定も設定可能
    // ============================================================
    struct SpawnConfig
    {
        float minDist = 200.0f;     // プレイヤーからの最小スポーン距離
        float maxDist = 400.0f;     // プレイヤーからの最大スポーン距離
        float interval = 2.0f;      // スポーン間隔（秒）
        int   maxAlive = 10;        // 同時に存在可能な最大数
        float weight = 1.0f;        // ランダム抽選の重み（確率）

        const char* texture = "asset/Texture/NormalEnemy.png"; // 使用テクスチャ
        float sizeX = 100.0f;       // サイズX
        float sizeY = 100.0f;       // サイズY
        float collisionRadius = 50.0f; // 当たり判定半径

        float stopDist = 0.0f;      // （任意）スポーン後の追跡停止距離（0なら最後まで追跡）

        int sheetX = 1;             // スプライトシート分割数（横）
        int sheetY = 1;             // スプライトシート分割数（縦）
        bool useSheet = false;      // スプライトシートを使うか

        bool useAnim = false;       // アニメを使うか
        Animation anim{ 0, 1, 0.1f, true }; // アニメ定義（開始/枚数/速度/ループ）

        // テクスチャ設定
        void SetTexture(const char* path) { texture = path; }

        // スプライトシート設定
        void SetSpriteSheet(int x, int y) { sheetX = x; sheetY = y; useSheet = true; }

        // アニメ設定
        void SetAnim(int start, int count, float frameTime, bool loop)
        {
            anim = { start, count, frameTime, loop };
            useAnim = true;
        }
    };

    // Animator 取得（EnemySpawner や描画側で参照可能）
    Animator& GetAnimator() { return m_animator; }
    const Animator& GetAnimator() const { return m_animator; }

    // DrawScene で敵の現在フレームを描画するために使用
    int GetAnimFrame() const { return m_animator.GetCurrentFrame(); }

    // スポーン直後（Object と紐付け後）に 1 回呼んで Idle 開始
    void OnSpawned();

    // 各敵タイプが自分のスポーン設定を返す（派生クラスで override 推奨）
    virtual SpawnConfig GetSpawnConfig() const { return SpawnConfig(); }

   
protected:
    bool isBoss = false; // ボス判定（必要なら派生で利用）

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
    float knockBackTimer = 0.0f;          // ノックバック経過時間
    float knockBackDuration = 2.0f;      // ノックバック継続時間

    // =========================
    // アニメーション定義（派生クラスが SetupAnimation() で設定）
    // =========================
    Animation m_idleAnim{ 0, 1, 0.2f, true };   // 待機アニメ
    Animation m_walkAnim{ 0, 1, 0.1f, true };   // 移動アニメ
    Animation m_hitAnim{ 0, 1, 0.08f, false }; // 被弾アニメ（ループなし）

    bool m_textureRightFacing = false; // テクスチャ原画が右向きか（反転判定用）

    // 派生クラスで必ずアニメ範囲（フレーム）を設定する
    virtual void SetupAnimation() = 0;

    // 最低限必要：歩き見た目（テクスチャ/シート）を適用する
    virtual void ApplyWalkVisual() = 0;

    // 既定は Walk と同じ（必要なら派生 cpp でオーバーライド可能）
    virtual void ApplyIdleVisual() { ApplyWalkVisual(); }
    virtual void ApplyHitVisual() { ApplyWalkVisual(); }

    // 被弾時の処理フック（派生 cpp 側で管理できる）
    virtual void OnDamaged(int damage);

    // 現在のアニメ状態
    enum class AnimState { None, Idle, Walk, Hit };
    AnimState m_animState = AnimState::None;

    // 状態に応じたアニメ再生
    void PlayIdle();
    void PlayWalk();
    void PlayHit();

    // Enemy用 Animator
    Animator m_animator;

    // 補助フラグ
    bool m_isWalking = false; // 移動中か
    bool m_facingRight = true; // 現在向き（右向きなら true）
};
