#include "EnemySpawner.h"
#include <cmath> // sqrtf / sinf / cosf
#include "Scene.h"
#include "Object.h"
#include "Enemy.h"
#include <algorithm>
#include <unordered_map>
#include "Boss.h" 
#include "Player.h" 
#include "GamePlay.h"

EnemySpawner::EnemySpawner()
{
    std::random_device rd;
    m_rng = std::mt19937(rd());
}

void EnemySpawner::Init(Scene* ownerScene, Object* playerObj, Player* player)
{
    m_scene = ownerScene;
    m_player = playerObj;
    m_playerLogic = player;

    m_enemies.clear();
    m_entries.clear();
    m_spawnTimer = 0.0f;

    m_killCount = 0;        // ✅ 初期化：キル数
    m_bossSpawned = false;  // ✅ 初期化：ボス出現フラグ

}

void EnemySpawner::Update(float deltaTime)
{
    if (!m_scene || !m_player) return;
    if (m_entries.empty()) return;

    // (1) update enemies
    for (auto& e : m_enemies)
    {
        if (e) e->Update(deltaTime);
    }

    // cleanup dead enemies
    CleanupDeadEnemies();

    // (1.5) enemy-enemy collision pushout
    // stopDistで止まっている敵は「押し出しで動かさない」
    for (int iter = 0; iter < 3; ++iter)
    {
        ResolveEnemyCollisions();
    }

    // (2) spawn timer
    m_spawnTimer += deltaTime;

    int idx = RandIndexByWeight();
    if (idx < 0 || idx >= (int)m_entries.size()) return;

    std::unique_ptr<Enemy> temp = m_entries[idx].createFn();
    if (!temp) return;
    Enemy::SpawnConfig cfg = temp->GetSpawnConfig();

    if ((int)m_enemies.size() >= cfg.maxAlive) return;

    if (m_spawnTimer >= cfg.interval)
    {
        m_spawnTimer = 0.0f;

        std::unique_ptr<Enemy> enemy = m_entries[idx].createFn();
        if (!enemy) return;

        cfg = enemy->GetSpawnConfig();

        Object* obj = m_scene->AddObject();
        if (!obj) return;

        obj->Init(cfg.texture, cfg.sheetX, cfg.sheetY);
        if (cfg.useSheet)
        {
            obj->SetSpriteSheet(cfg.sheetX, cfg.sheetY);
        }

        obj->SetSize(cfg.sizeX, cfg.sizeY, 0.0f);
        obj->SetCollisionRadius(cfg.collisionRadius);

        auto pp3 = m_player->GetPos();
        DirectX::SimpleMath::Vector2 center(pp3.x, pp3.y);

        float angle = RandFloat(0.0f, 6.2831853f);
        float dist = RandFloat(cfg.minDist, cfg.maxDist);

        float sx = center.x + cosf(angle) * dist;
        float sy = center.y + sinf(angle) * dist;
        obj->SetPos(sx, sy, 0.0f);

        enemy->SetObject(obj);
        enemy->SetTarget(m_player);

        enemy->SetGamePlay(
            static_cast<GamePlay*>(m_scene)
        );

        // ✅ 死亡演出時間（SpawnConfigを反映）
        enemy->SetDeathDelay(cfg.dieDelay);
        enemy->SetDisappearDelay(cfg.disappearDelay);

        enemy->OnSpawned();

        enemy->SetChaseEnabled(true);

        if (cfg.stopDist > 0.0f) enemy->SetChaseStopDistance(cfg.stopDist);
        else                    enemy->SetChaseStopDistance(0.0f);

        m_enemies.emplace_back(std::move(enemy));
    }
}

float EnemySpawner::RandFloat(float a, float b)
{
    std::uniform_real_distribution<float> dist(a, b);
    return dist(m_rng);
}

int EnemySpawner::RandIndexByWeight()
{
    if (m_entries.empty()) return -1;

    float total = 0.0f;
    for (auto& e : m_entries) total += e.weight;

    if (total <= 0.0f) return 0;

    float r = RandFloat(0.0f, total);
    float acc = 0.0f;

    for (int i = 0; i < (int)m_entries.size(); ++i)
    {
        acc += m_entries[i].weight;
        if (r <= acc) return i;
    }
    return (int)m_entries.size() - 1;
}

void EnemySpawner::ResolveEnemyCollisions()
{
    if (m_enemies.size() < 2) return;
    if (!m_player) return;

    // プレイヤー位置（stopDist 判定用）
    auto pp3 = m_player->GetPos();
    DirectX::SimpleMath::Vector2 playerPos(pp3.x, pp3.y);

    // stopDist に到達して止まっている敵は、押し出しで動かさない
    // ※ノックバック中は例外（飛んでいる敵は固定しない）
    auto IsStoppedEnemy = [&](Enemy* e, Object* eo) -> bool
        {
            if (!e || !eo) return false;

            const float stop = e->GetChaseStopDistance();
            if (stop <= 0.0f) return false;

            if (e->IsKnockBacking()) return false;

            auto p3 = eo->GetPos();
            DirectX::SimpleMath::Vector2 pos(p3.x, p3.y);
            float dist = (pos - playerPos).Length();

            // 境界でガタつかないよう少し余裕
            return dist <= (stop + 2.0f);
        };

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

            const bool aFly = a->IsKnockBacking();
            const bool bFly = b->IsKnockBacking();

            auto pa3 = oa->GetPos();
            auto pb3 = ob->GetPos();

            float dx = pb3.x - pa3.x;
            float dy = pb3.y - pa3.y;

            float distSq = dx * dx + dy * dy;
            float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

            float nx = dx / dist;
            float ny = dy / dist;

            // ---- impact damage（既存）----
            if (aFly && !bFly)
            {
                int impact = 0;
                if (a->TryConsumeImpactDamage(impact))
                {
                    b->TakeDamage(impact);
                }
            }
            else if (!aFly && bFly)
            {
                int impact = 0;
                if (b->TryConsumeImpactDamage(impact))
                {
                    a->TakeDamage(impact);
                }
            }

            // ---- pushout ----
            float ra = oa->GetCollisionRadius();
            float rb = ob->GetCollisionRadius();

            float overlap = (ra + rb) - dist;
            if (overlap <= 0.0f) continue;

            bool aStop = IsStoppedEnemy(a, oa);
            bool bStop = IsStoppedEnemy(b, ob);

            float pushA = overlap * 0.5f;
            float pushB = overlap * 0.5f;

            if (aStop && !bStop)
            {
                pushA = 0.0f;
                pushB = overlap;
            }
            else if (!aStop && bStop)
            {
                pushA = overlap;
                pushB = 0.0f;
            }
            else if (aStop && bStop)
            {
                // 両方止まっている場合：プレイヤー中心の円周接線方向へ逃がす（押し込みにくくする）
                DirectX::SimpleMath::Vector2 posA(pa3.x, pa3.y);
                DirectX::SimpleMath::Vector2 posB(pb3.x, pb3.y);

                auto mid = (posA + posB) * 0.5f;
                auto r = mid - playerPos;
                float rlen = r.Length();

                if (rlen < 0.0001f) r = { 1.0f, 0.0f };
                else r /= rlen;

                DirectX::SimpleMath::Vector2 t(-r.y, r.x);

                auto ab = (posB - posA);
                if (ab.Dot(t) < 0.0f) t = -t;

                float ax = -t.x * pushA;
                float ay = -t.y * pushA;
                float bx = t.x * pushB;
                float by = t.y * pushB;

                oa->SetPos(pa3.x + ax, pa3.y + ay, pa3.z);
                ob->SetPos(pb3.x + bx, pb3.y + by, pb3.z);
                continue;
            }

            oa->SetPos(pa3.x - nx * pushA, pa3.y - ny * pushA, pa3.z);
            ob->SetPos(pb3.x + nx * pushB, pb3.y + ny * pushB, pb3.z);
        }
    }
}

void EnemySpawner::CleanupDeadEnemies()
{
    for (auto it = m_enemies.begin(); it != m_enemies.end(); )
    {
        Enemy* e = it->get();
        if (!e)
        {
            it = m_enemies.erase(it);
            continue;
        }

        if (!e->IsAlive())
        {
            if (!e->IsRewardGiven())
            {
                if (m_playerLogic)
                {
                    m_playerLogic->AddExp(e->GetExpValue());
                }

                e->MarkRewardGiven();
            }
            // ✅ 雑魚だけ討伐数を加算（ボスはカウントしない）
            if (!e->IsBoss())
            {
                m_killCount++;

                if (m_killCount >= 10 && !m_bossSpawned)
                {
                    SpawnBoss();
                }
            }

            if (Object* o = e->GetObject())
            {
                o->SetCollisionRadius(0.0f);
                o->SetColor(1, 1, 1, 0.0f);
                auto p = o->GetPos();
                o->SetPos(999999.0f, 999999.0f, p.z);
            }

            it = m_enemies.erase(it);
            continue;
        }

        ++it;
    }
}

void EnemySpawner::SpawnBoss()
{
    if (m_bossSpawned) return;
    if (!m_scene || !m_player) return;

    std::unique_ptr<Enemy> boss = std::make_unique<Boss>();
    if (!boss) return;

    Enemy::SpawnConfig cfg = boss->GetSpawnConfig();

    Object* obj = m_scene->AddObject();
    if (!obj) return;

    obj->Init(cfg.texture, cfg.sheetX, cfg.sheetY);
    if (cfg.useSheet)
        obj->SetSpriteSheet(cfg.sheetX, cfg.sheetY);

    obj->SetSize(cfg.sizeX, cfg.sizeY, 0.0f);
    obj->SetCollisionRadius(cfg.collisionRadius);

    auto pp3 = m_player->GetPos();
    DirectX::SimpleMath::Vector2 center(pp3.x, pp3.y);

    float angle = RandFloat(0.0f, 6.2831853f);
    float dist = RandFloat(cfg.minDist, cfg.maxDist);

    float sx = center.x + cosf(angle) * dist;
    float sy = center.y + sinf(angle) * dist;
    obj->SetPos(sx, sy, 0.0f);

    boss->SetObject(obj);
    boss->SetTarget(m_player);
    boss->SetGamePlay(
        static_cast<GamePlay*>(m_scene)
    );

    boss->SetDeathDelay(cfg.dieDelay);
    boss->SetDisappearDelay(cfg.disappearDelay);

    boss->OnSpawned();

    boss->SetChaseEnabled(true);
    if (cfg.stopDist > 0.0f) boss->SetChaseStopDistance(cfg.stopDist);

    m_enemies.emplace_back(std::move(boss));
    m_bossSpawned = true;
}