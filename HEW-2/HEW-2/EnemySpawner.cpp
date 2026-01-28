#include "EnemySpawner.h"
#include <cmath> // sqrtf / sinf / cosf 用
#include "Scene.h"
#include "Object.h"
#include "Enemy.h"

EnemySpawner::EnemySpawner()
{
    std::random_device rd;
    m_rng = std::mt19937(rd());
}

void EnemySpawner::Init(Scene* ownerScene, Object* playerObj)
{
    // シーン（AddObject を使うため）とプレイヤーObjectを保持
    m_scene = ownerScene;
    m_player = playerObj;

    // 管理している敵ロジック/登録タイプ/タイマーを初期化
    m_enemies.clear();
    m_entries.clear();
    m_spawnTimer = 0.0f;
}

void EnemySpawner::Update(float deltaTime)
{
    // シーンまたはプレイヤーが無ければ何もしない
    if (!m_scene || !m_player) return;

    // 生成する敵タイプが登録されていなければ何もしない
    if (m_entries.empty()) return;

    // =========================
    // (1) 既存の敵を毎フレーム更新
    //     （追跡/移動/AI などは Enemy::Update 内で行われる）
    // =========================
    for (auto& e : m_enemies)
    {
        if (e) e->Update(deltaTime);
    }

    // =========================
    // (1.5) 敵同士の当たり判定（押し出し）
    // =========================
    ResolveEnemyCollisions();

    // =========================
    // (2) スポーン用タイマー更新
    //     敵タイプごとに interval が違うため、先に「今回スポーンするタイプ」を決める
    // =========================
    m_spawnTimer += deltaTime;

    // 今回スポーンする敵タイプのインデックスを抽選
    int idx = RandIndexByWeight();
    if (idx < 0 || idx >= (int)m_entries.size()) return;

    // -------------------------
    // ※注意：
    // ここで一時インスタンスを作って GetSpawnConfig() を読むため、
    // タイプ数が多い/毎フレーム呼ぶと重くなり得る（最適化は後で可能）
    // -------------------------
    std::unique_ptr<Enemy> temp = m_entries[idx].createFn();
    Enemy::SpawnConfig cfg = temp->GetSpawnConfig();

    // 同時に存在できる最大数に達していたらスポーンしない
    if ((int)m_enemies.size() >= cfg.maxAlive) return;

    // interval 秒を超えたらスポーン
    if (m_spawnTimer >= cfg.interval)
    {
        m_spawnTimer = 0.0f;

        // =========================
        // 実際の敵ロジック生成（同じ idx のタイプ）
        // =========================
        std::unique_ptr<Enemy> enemy = m_entries[idx].createFn();
        if (!enemy) return;

        // この敵タイプのスポーン設定を取得
        cfg = enemy->GetSpawnConfig();

        // =========================
        // Object を生成（Scene が所有）
        // =========================
        Object* obj = m_scene->AddObject();
        if (!obj) return;

        // 生成する Object の見た目/サイズ/当たり判定
        obj->Init(cfg.texture);
        obj->SetSize(cfg.sizeX, cfg.sizeY, 0.0f);
        obj->SetCollisionRadius(cfg.collisionRadius);

        // =========================
        // スポーン位置：プレイヤー中心の「円環(minDist～maxDist)」でランダム
        // =========================
        auto pp3 = m_player->GetPos();
        DirectX::SimpleMath::Vector2 center(pp3.x, pp3.y);

        float angle = RandFloat(0.0f, 6.2831853f); // 0～2π
        float dist = RandFloat(cfg.minDist, cfg.maxDist);

        float sx = center.x + cosf(angle) * dist;
        float sy = center.y + sinf(angle) * dist;

        obj->SetPos(sx, sy, 0.0f);

        // =========================
        // ロジックと Object を接続
        // =========================
        enemy->SetObject(obj);
        enemy->SetTarget(m_player);

        // =========================
        // 追跡設定（念のため明示）
        // =========================
        enemy->SetChaseEnabled(true);

        // stopDist が 0 より大きければ、その距離で追跡を止める
        // 0 の場合は最後まで追跡
        if (cfg.stopDist > 0.0f) enemy->SetChaseStopDistance(cfg.stopDist);
        else                    enemy->SetChaseStopDistance(0.0f);

        // 管理リストに追加（EnemySpawner がロジックを所有）
        m_enemies.emplace_back(std::move(enemy));
    }
}

float EnemySpawner::RandFloat(float a, float b)
{
    // a～b の一様乱数
    std::uniform_real_distribution<float> dist(a, b);
    return dist(m_rng);
}

int EnemySpawner::RandIndexByWeight()
{
    // 登録が無いなら無効
    if (m_entries.empty()) return -1;

    // 重み合計
    float total = 0.0f;
    for (auto& e : m_entries) total += e.weight;

    // 合計が 0 以下なら 0 を返す
    if (total <= 0.0f) return 0;

    // 0～total の範囲で乱数を引いて累積で決定
    float r = RandFloat(0.0f, total);
    float acc = 0.0f;

    for (int i = 0; i < (int)m_entries.size(); ++i)
    {
        acc += m_entries[i].weight;
        if (r <= acc) return i;
    }
    return (int)m_entries.size() - 1;
}

// ※ここで <cmath> を二重 include しているなら片方は消してOK
// #include <cmath> // sqrtf
void EnemySpawner::ResolveEnemyCollisions()
{
    // 敵が2体未満なら、衝突判定する必要なし
    if (m_enemies.size() < 2) return;

    // =========================
    // 全ての敵ペア(i < j)で衝突チェック
    // =========================
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy* a = m_enemies[i].get();
        if (!a) continue;

        Object* oa = a->GetObject();
        if (!oa) continue;

        for (size_t j = i + 1; j < m_enemies.size(); ++j)
        {
            Enemy* b = m_enemies[j].get();
            if (!b) continue;

            Object* ob = b->GetObject();
            if (!ob) continue;

            // 円同士の当たり判定（重なっていなければ処理しない）
            if (!oa->CheckCollision(*ob)) continue;

            // 2体の座標を取得
            auto pa3 = oa->GetPos();
            auto pb3 = ob->GetPos();

            // a -> b の差分ベクトル
            float dx = pb3.x - pa3.x;
            float dy = pb3.y - pa3.y;

            // 距離計算（0割防止のため最小値を入れる）
            float distSq = dx * dx + dy * dy;
            float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

            // =========================
            // 各Objectに設定されている当たり判定半径を使用
            // （SetCollisionRadius / SetSize によって決まる）
            // =========================
            float ra = oa->GetCollisionRadius();
            float rb = ob->GetCollisionRadius();

            // 重なり量 = (半径の合計) - (中心距離)
            float overlap = (ra + rb) - dist;
            if (overlap <= 0.0f) continue;

            // 押し出し方向（正規化）
            float nx = dx / dist;
            float ny = dy / dist;

            // お互いを半分ずつ押し出す
            float push = overlap * 0.5f;

            // =========================
            // 瞬間移動っぽく見えるのを防ぐため、
            // 1フレームでの押し出し量に上限を設ける
            // =========================
            const float maxPush = 4.0f;
            if (push > maxPush) push = maxPush;

            // a は -方向へ、b は +方向へ移動させて重なりを解消
            oa->SetPos(pa3.x - nx * push, pa3.y - ny * push, pa3.z);
            ob->SetPos(pb3.x + nx * push, pb3.y + ny * push, pb3.z);
        }
    }
}
