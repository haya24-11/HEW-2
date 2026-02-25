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

    // 敵タイプ登録（複数タイプ登録可能）
    template<typename T>
    void RegisterType(float weight = 1.0f);

    // 毎フレーム呼び出し
    void Update(float deltaTime);

    // 衝突チェック用（必要なら GamePlay 側で利用）
    const std::vector<std::unique_ptr<Enemy>>& GetEnemies() const { return m_enemies; }

    // ★ デバッグ / 即スポーン用
   // Enemy* SpawnOneImmediate();

   

private:
 //   Enemy* SpawnOne(); // 1体スポーン

    // 乱数ユーティリティ
    float RandFloat(float a, float b);
    int   RandIndexByWeight();

    // 敵-敵の衝突処理（重なったら押し出す）
    void ResolveEnemyCollisions();
private:
    Scene* m_scene = nullptr;
    Object* m_player = nullptr;
    Player* m_playerLogic = nullptr;

    // 登録された敵タイプ（ファクトリ）
    struct Entry
    {
        float weight = 1.0f;                        // 抽選の重み（確率）
        std::unique_ptr<Enemy>(*createFn)();        // 生成関数（Enemy派生を作る）
    };
    std::vector<Entry> m_entries;

    // 生存している敵（ロジックを EnemySpawner が所有）
    std::vector<std::unique_ptr<Enemy>> m_enemies;

    // スポーン制御（タイプ別 config によるが、現在選ばれたタイプの interval を使用）
    float m_spawnTimer = 0.0f;

    // 乱数エンジン
    std::mt19937 m_rng;

    void CleanupDeadEnemies();

    int  m_killCount = 0;        
    bool m_bossSpawned = false;  


    void SpawnBoss();            // ✅ ボスを強制スポーン
};

// =========================
// template の実装はヘッダに書く必要がある
// =========================
template<typename T>
void EnemySpawner::RegisterType(float weight)
{
    Entry e;
    e.weight = (weight <= 0.0f) ? 0.0f : weight;

    // T（Enemy派生）を生成するラムダを登録
    e.createFn = []() -> std::unique_ptr<Enemy>
        {
            return std::make_unique<T>();
        };

    // 登録リストに追加
    m_entries.push_back(e);
}
