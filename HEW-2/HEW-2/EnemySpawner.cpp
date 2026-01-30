#include "EnemySpawner.h"
#include <cmath> // sqrtf / sinf / cosf 用
#include "Scene.h"
#include "Object.h"
#include "Enemy.h"
#include <algorithm> // std::erase_if
#include <unordered_map>

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

    CleanupDeadEnemies();

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
        obj->Init(cfg.texture, cfg.sheetX, cfg.sheetY);

        // （保険）もし Init 内で SetSpriteSheet を呼ばない構成なら、
        // 下の1行も有効にしておくと確実
        if (cfg.useSheet)
        {
            obj->SetSpriteSheet(cfg.sheetX, cfg.sheetY);
        }

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

        enemy->OnSpawned();

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
    if (m_enemies.size() < 2) return;

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

            if (!oa->CheckCollision(*ob)) continue;

            // =====================================================
            // ✅ （追加）強攻撃で飛んでいる敵が他の敵にぶつかったらダメージを与える
            // =====================================================
            const bool aFly = a->IsKnockBacking();
            const bool bFly = b->IsKnockBacking();

            // 法線（a -> b）を計算
            auto pa3 = oa->GetPos();
            auto pb3 = ob->GetPos();

            float dx = pb3.x - pa3.x;
            float dy = pb3.y - pa3.y;

            float distSq = dx * dx + dy * dy;
            float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

            float nx = dx / dist;
            float ny = dy / dist;

            // a が飛んでいて b が歩行中なら：a のインパクトダメージを b に与える
            if (aFly && !bFly)
            {
                int impact = 0;
                if (a->TryConsumeImpactDamage(impact))
                {
                    b->TakeDamage(impact);
                }
            }
            // b が飛んでいて a が歩行中なら：b のインパクトダメージを a に与える
            else if (!aFly && bFly)
            {
                int impact = 0;
                if (b->TryConsumeImpactDamage(impact))
                {
                    a->TakeDamage(impact);
                }
            }
            // （両方飛んでいる場合）必要なら相互にダメージを与える処理も可能
            // else if (aFly && bFly) { ... }

            // =====================================================
            // 既存：重なり解消（押し出し）
            // =====================================================
            float ra = oa->GetCollisionRadius();
            float rb = ob->GetCollisionRadius();

            float overlap = (ra + rb) - dist;
            if (overlap <= 0.0f) continue;

            float push = overlap * 0.5f;

            oa->SetPos(pa3.x - nx * push, pa3.y - ny * push, pa3.z);
            ob->SetPos(pb3.x + nx * push, pb3.y + ny * push, pb3.z);
        }
    }
}

void EnemySpawner::CleanupDeadEnemies()
{
    std::erase_if(m_enemies, [&](const std::unique_ptr<Enemy>& e)
        {
            if (!e) return true;

            // Enemy/Chara 側で hp<=0 になったら isAlive=false にしている前提
            if (!e->IsAlive())
            {
                // Object を「完全削除」できない構造なら、最低限 無効化しておく
                if (Object* o = e->GetObject())
                {
                    o->SetCollisionRadius(0.0f);
                    o->SetColor(1, 1, 1, 0.0f);
                    auto p = o->GetPos();
                    o->SetPos(999999.0f, 999999.0f, p.z);
                }
                return true; // ✅ リストから除去（sizeが減るのでリスポーン可能になる）
            }
            return false;
        });
}
