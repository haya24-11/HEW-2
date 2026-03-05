#pragma once
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <SimpleMath.h>

class Scene;
class Object;
class Enemy;
class Player;

class EnemySpawner
{
public:
    EnemySpawner();

    // Scene（AddObject を使うため）と Player（Object*）を受け取って初期化
    void Init(Scene* ownerScene, Object* playerObj, Player* player);

    // ✅ 敵タイプ登録：TのGetSpawnConfig()から interval / maxAlive を読む
    template<class T>
    void RegisterType(float weight = 1.0f)
    {
        Entry e;
        e.weight = (weight <= 0.0f) ? 0.0f : weight;

        // 日本語コメント：TのSpawnConfigから interval/maxAlive を読む
        T tmp;
        auto cfg = tmp.GetSpawnConfig();

        e.interval = (cfg.interval <= 0.0f) ? 0.001f : cfg.interval;
        e.maxAlive = (cfg.maxAlive < 0) ? 0 : cfg.maxAlive;

        e.timer = 0.0f;
        e.aliveCount = 0;

        e.createFn = []() -> std::unique_ptr<Enemy>
            {
                return std::make_unique<T>();
            };

        m_entries.push_back(std::move(e));
    }

    // 毎フレーム呼び出し
    void Update(float deltaTime);

    // 衝突チェック用（必要なら GamePlay 側で利用）
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return m_enemies; }

    // Result掲載用
    int GetKillCount() const { return m_killCount; }

private:
    // 乱数ユーティリティ
    float RandFloat(float a, float b);

    // 重み抽選（使うなら）
    int RandIndexByWeight();

    // 敵-敵の衝突処理（重なったら押し出す）
    void ResolveEnemyCollisions();

    // 死亡した敵の掃除
    void CleanupDeadEnemies();

    void SpawnBoss(); // ✅ ボスを強制スポーン

private:
    Scene* m_scene = nullptr;
    Object* m_player = nullptr;
    Player* m_playerLogic = nullptr;

    // 登録された敵タイプ（ファクトリ + タイプ別設定）
    struct Entry
    {
        float weight = 1.0f;
        std::unique_ptr<Enemy>(*createFn)() = nullptr;

        // タイプ別スポーン設定
        float interval = 1.0f;
        int   maxAlive = 10;

        // タイプ別状態
        float timer = 0.0f;
        int   aliveCount = 0;
    };

    std::vector<Entry> m_entries;

    // 生存している敵（ロジックを EnemySpawner が所有）
    std::vector<std::unique_ptr<Enemy>> m_enemies;

    // 乱数エンジン
    std::mt19937 m_rng;

    int  m_killCount = 0;
    bool m_bossSpawned = false;

    std::vector<unsigned char> m_stopLock; // ★このフレーム「外にいた敵」をロック

    // =========================
    // Impact衝突エフェクト管理
// =========================
    struct ImpactFX
    {
        Object* obj = nullptr;
        float   t = 0.0f;
        int     frame = 0;
    };

    std::vector<ImpactFX> m_impactFX;

    void SpawnImpactEffect(float x, float y);
    void UpdateImpactEffects(float dt);
};