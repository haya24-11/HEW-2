#pragma once
#include "Chara.h"
#include "Object.h"
#include "Animator.h"
#include <SimpleMath.h>

class GamePlay;
/*
    Enemy
    =====
    ・敵キャラ共通処理
    ・追跡/ノックバック/被弾アニメ
    ・被弾時の赤点滅（ヒットフラッシュ）
    ・死亡：すぐ消さず、死亡モーションを一定時間出してから消す
*/

class Enemy : public Chara
{
public:
    Enemy();
    virtual ~Enemy() = default;

    void Update(float deltaTime) override;

    // ✅ CharaのTakeDamageを上書き（被弾演出＋死亡予約など）
    void TakeDamage(int damage) override;

    void SetTarget(Object* target) { m_target = target; }

    void SetChaseStopDistance(float d) { chaseStopDistance = d; }
    float GetChaseStopDistance() const { return chaseStopDistance; }

    void SetChaseEnabled(bool enabled) { chaseEnabled = enabled; }
    bool IsChaseEnabled() const { return chaseEnabled; }

    void KnockBack(const DirectX::SimpleMath::Vector2& force);

    bool IsKnockBacking() const { return knockBackTimer > 0.0f; }

    void StopKnockBack()
    {
        knockBackTimer = 0.0f;
        knockBackVelocity = { 0.0f, 0.0f };
    }

    // ✅ 強攻撃など「吹き飛び終わってから死ぬ」をON/OFF
    void EnableDeathAfterKnockback(bool enable) { m_deathAfterKnockback = enable; }

    // ✅ 死亡演出の時間を外から調整できるようにする
    // 「死亡モーションを出す時間（基本 0.3秒）」
    void SetDeathDelay(float sec) { m_dieDelay = (sec < 0.0f) ? 0.0f : sec; }
    float GetDeathDelay() const { return m_dieDelay; }

    // 「死亡後、さらに何秒残してから消すか（任意）」
    void SetDisappearDelay(float sec) { m_disappearDelay = (sec < 0.0f) ? 0.0f : sec; }
    float GetDisappearDelay() const { return m_disappearDelay; }

    // =========================
    // ✅ Spawner（タイプ別aliveCount管理用）
    // =========================
    // 日本語コメント：この敵がどのEntry（タイプ）から生成されたか
    void SetSpawnerEntryIndex(int idx) { m_spawnerEntryIndex = idx; }
    int  GetSpawnerEntryIndex() const { return m_spawnerEntryIndex; }

    // =========================
    // SpawnConfig
    // =========================
    struct SpawnConfig
    {
        float minDist = 200.0f;
        float maxDist = 400.0f;
        float interval = 2.0f;
        int   maxAlive = 10;
        float weight = 1.0f;

        const char* texture = "asset/Texture/NormalEnemy.png";
        float sizeX = 100.0f;
        float sizeY = 100.0f;
        float collisionRadius = 50.0f;

        float stopDist = 0.0f;

        int hp = 30;
        int power = 5;

        int sheetX = 1;
        int sheetY = 1;
        bool useSheet = false;

        bool useAnim = false;
        Animation anim{ 0, 1, 0.1f, true };

        // ✅ 死亡演出設定（必要ならSpawner側で反映）
        float dieDelay = 0.30f;       // 死亡モーションを見せる時間
        float disappearDelay = 0.00f; // さらに残す時間（任意）

        void SetTexture(const char* path) { texture = path; }
        void SetSpriteSheet(int x, int y) { sheetX = x; sheetY = y; useSheet = true; }

        void SetAnim(int start, int count, float frameTime, bool loop)
        {
            anim = { start, count, frameTime, loop };
            useAnim = true;
        }
    };

    Animator& GetAnimator() { return m_animator; }
    const Animator& GetAnimator() const { return m_animator; }
    int GetAnimFrame() const { return m_animator.GetCurrentFrame(); }

    void OnSpawned();
    virtual SpawnConfig GetSpawnConfig() const { return SpawnConfig(); }

    // ✅ 飛んでいる敵が他の敵にぶつかった時に与える「インパクトダメージ」を設定
    void SetImpactDamage(int dmg) { m_impactDamage = (dmg < 0) ? 0 : dmg; }

    // ✅ インパクトダメージを取得
    int GetImpactDamage() const { return m_impactDamage; }

    // ✅ 連続ヒット防止：準備できていればダメージを1回だけ取り出して適用する
    bool TryConsumeImpactDamage(int& outDmg)
    {
        if (m_impactDamage <= 0) return false;
        if (m_impactCooldown > 0.0f) return false;

        outDmg = m_impactDamage;

        // 同じ敵へ毎フレーム当たってダメージが入り続けるのを防ぐ
        m_impactCooldown = 0.20; // 0.08～0.20 推奨
        return true;
    }

    bool IsBoss() const { return isBoss; }

    // EXP用
    int GetExpValue() const { return m_expValue; }

    bool IsRewardGiven() const { return m_rewardGiven; }
    void MarkRewardGiven() { m_rewardGiven = true; }

    void SetGamePlay(GamePlay* gp) { m_gamePlay = gp; }
    GamePlay* GetGamePlay() const { return m_gamePlay; }

    DirectX::SimpleMath::Vector2 GetKnockBackVelocity() const { return knockBackVelocity; }
    void SetKnockBackVelocity(const DirectX::SimpleMath::Vector2& v) { knockBackVelocity = v; }

    // 衝突反動用：短いノックバックを付与（速度を加算）
    void AddKnockBackImpulse(const DirectX::SimpleMath::Vector2& v, float sec);

protected:
    bool isBoss = false;

    Object* m_target = nullptr;
    float chaseStopDistance = 0.0f;
    bool chaseEnabled = true;

    // ノックバック
    DirectX::SimpleMath::Vector2 knockBackVelocity{ 0.0f, 0.0f };
    float knockBackTimer = 0.0f;
    float knockBackDuration = 2.0f;

    // アニメ
    Animation m_idleAnim{ 0, 1, 0.2f, true };
    Animation m_walkAnim{ 0, 1, 0.1f, true };
    Animation m_hitAnim{ 0, 1, 0.08f, false };
    Animation m_dieAnim{ 0, 24, 0.5f, false }; // ✅ 死亡モーション用（派生クラスで設定してね）

    bool m_textureRightFacing = false;

    virtual void SetupAnimation() = 0;
    virtual void ApplyWalkVisual() = 0;

    virtual void ApplyIdleVisual() { ApplyWalkVisual(); }
    virtual void ApplyHitVisual() { ApplyWalkVisual(); }
    virtual void ApplyDieVisual() { ApplyWalkVisual(); } // ✅ 死亡用の見た目（必要なら派生でoverride）

    virtual void OnDamaged(int damage);

    enum class AnimState { None, Idle, Walk, Hit, Die };
    AnimState m_animState = AnimState::None;

    void PlayIdle();
    void PlayWalk();
    void PlayHit();
    void PlayDie();

    Animator m_animator;

    bool m_isWalking = false;
    bool m_facingRight = true;

    // 被弾演出（赤点滅）
    void StartHitFlash(float sec);
    void UpdateHitFlash(float dt);

    // ✅ HPが0になった瞬間に呼ばれる（Chara::TakeDamageから）
    void OnHpZero() override;

    // ✅ 「死亡演出開始」と「最終的に消す」
    void BeginDeath();
    void DieNow();

    int m_expValue = 10; // デフォルト経験値

private:
    // ✅ Spawner（タイプ別aliveCount管理用）
    int m_spawnerEntryIndex = -1;

    // ヒットフラッシュ
    float m_hitFlashTimer = 0.0f;
    float m_hitFlashDuration = 0.10f;

    // 強攻撃用：ノックバック後に死ぬ
    bool m_deathAfterKnockback = false;
    bool m_pendingDeath = false;

    // ✅ 死亡演出タイマー
    float m_dieDelay = 100.0f;   // 死亡モーションを見せる時間
    float m_dieTimer = 100.0f;

    bool  m_isDying = false;

    float m_disappearDelay = 10.0f;  // 死亡後さらに残す（任意）
    float m_disappearTimer = 10.0f;

    int   m_impactDamage = 0;      // 飛んでいる敵が衝突で与えるダメージ
    float m_impactCooldown = 0.0f; // 連続ヒット防止用のクールタイム

    bool m_rewardGiven = false;

    GamePlay* m_gamePlay = nullptr;
};