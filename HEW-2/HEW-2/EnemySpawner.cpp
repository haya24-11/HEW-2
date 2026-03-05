#include "EnemySpawner.h"
#include <cmath>
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

    m_killCount = 0;
    m_bossSpawned = false;

    // ★念のため同期
    m_stopLock.assign(m_enemies.size(), 0);
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

        enemy->SetGamePlay(static_cast<GamePlay*>(m_scene));

        enemy->SetDeathDelay(cfg.dieDelay);
        enemy->SetDisappearDelay(cfg.disappearDelay);

        enemy->OnSpawned();

        enemy->SetChaseEnabled(true);

        if (cfg.stopDist > 0.0f) enemy->SetChaseStopDistance(cfg.stopDist);
        else                    enemy->SetChaseStopDistance(0.0f);

        m_enemies.emplace_back(std::move(enemy));

        // ★同期（保険）
        m_stopLock.assign(m_enemies.size(), 0);
    }
}

void EnemySpawner::DebugAddKill(int value)
{
    m_killCount += value;

    std::cout << "[DEBUG] KillCount = " << m_killCount << std::endl;
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

    using namespace DirectX::SimpleMath;

    // プレイヤー位置（stopDist 判定用）
    auto pp3 = m_player->GetPos();
    Vector2 playerPos(pp3.x, pp3.y);

    auto IsStoppedEnemy = [&](Enemy* e, Object* eo) -> bool
        {
            if (!e || !eo) return false;

            const float stop = e->GetChaseStopDistance();
            if (stop <= 0.0f) return false;

            // ノックバック中は「止まっている扱いにしない」
            if (e->IsKnockBacking()) return false;

            auto p3 = eo->GetPos();
            Vector2 pos(p3.x, p3.y);
            float dist = (pos - playerPos).Length();

            return dist <= (stop + 2.0f);
        };

    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy* a = m_enemies[i].get();
        if (!a) continue;
        if (!a->IsAlive()) continue;                // ✅ 追加：死んでる敵は処理しない

        Object* oa = a->GetObject();
        if (!oa) continue;

        for (size_t j = i + 1; j < m_enemies.size(); ++j)
        {
            Enemy* b = m_enemies[j].get();
            if (!b || !b->IsAlive()) continue;   // ✅ b 체크

            Object* ob = b->GetObject();
            if (!ob) continue;                   // ✅ ob 체크

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
            Vector2 n(nx, ny);

            // ---- impact damage ----
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

            // ---- pinball impulse ----
            {
                Vector2 va = aFly ? a->GetKnockBackVelocity() : Vector2(0.0f, 0.0f);
                Vector2 vb = bFly ? b->GetKnockBackVelocity() : Vector2(0.0f, 0.0f);

                float vrelN = (va - vb).Dot(n);

                if (vrelN > 0.0f)
                {
                    const float e = 0.85f;
                    const float transfer = 0.80f;
                    const float dampTangent = 0.20f;

                    const float kickTimeB = 0.35f;
                    const float minKickB = 180.0f;

                    auto KillTangent = [&](Vector2 v) -> Vector2
                        {
                            float vn = v.Dot(n);
                            Vector2 vN = vn * n;
                            Vector2 vT = v - vN;
                            vT *= (1.0f - dampTangent);
                            return vN + vT;
                        };

                    if (aFly)
                    {
                        Vector2 vReflect = va - (1.0f + e) * vrelN * n;
                        const float reflectMix = 0.55f;
                        Vector2 vaNew = va * (1.0f - reflectMix) + vReflect * reflectMix;
                        a->SetKnockBackVelocity(KillTangent(vaNew));
                    }

                    Vector2 impulseB = (1.0f + e) * vrelN * n * transfer;
                    Vector2 kickB = KillTangent(impulseB);

                    float len = kickB.Length();
                    if (len < minKickB)
                    {
                        if (len > 0.0001f) kickB *= (minKickB / len);
                        else               kickB = n * minKickB;
                    }

                    b->AddKnockBackImpulse(kickB, kickTimeB);
                }
            }

            // ---- position pushout ----
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
                Vector2 posA(pa3.x, pa3.y);
                Vector2 posB(pb3.x, pb3.y);

                auto mid = (posA + posB) * 0.5f;
                auto r = mid - playerPos;
                float rlen = r.Length();

                if (rlen < 0.0001f) r = { 1.0f, 0.0f };
                else r /= rlen;

                Vector2 t(-r.y, r.x);

                auto ab = (posB - posA);
                if (ab.Dot(t) < 0.0f) t = -t;

                oa->SetPos(pa3.x - t.x * pushA, pa3.y - t.y * pushA, pa3.z);
                ob->SetPos(pb3.x + t.x * pushB, pb3.y + t.y * pushB, pb3.z);
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

            if (!e->IsBoss())
            {
                m_killCount++;
                if (m_killCount >= 50 && !m_bossSpawned)
                {
                    SpawnBoss();
                }
            }

            if (Object* o = e->GetObject())
            {
                if (m_scene) m_scene->RemoveObject(o);
            }

            it = m_enemies.erase(it);
            continue;
        }

        ++it;
    }

    // ✅ enemy数と同期（安全）
    m_stopLock.assign(m_enemies.size(), 0);
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
    boss->SetGamePlay(static_cast<GamePlay*>(m_scene));

    boss->SetDeathDelay(cfg.dieDelay);
    boss->SetDisappearDelay(cfg.disappearDelay);

    boss->OnSpawned();

    boss->SetChaseEnabled(true);
    if (cfg.stopDist > 0.0f) boss->SetChaseStopDistance(cfg.stopDist);

    m_enemies.emplace_back(std::move(boss));
    m_bossSpawned = true;

    // ★同期（保険）
    m_stopLock.assign(m_enemies.size(), 0);
}