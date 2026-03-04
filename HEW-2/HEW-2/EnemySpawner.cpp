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

    using namespace DirectX::SimpleMath;

    // プレイヤー位置（stopDist 判定用）
    auto pp3 = m_player->GetPos();
    Vector2 playerPos(pp3.x, pp3.y);

    // stopDist に到達して止まっている敵は、押し出しで動かさない
    // ※ノックバック中は例外（飛んでいる敵は固定しない）
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

            // まずは当たり判定（円コリジョン）
            if (!oa->CheckCollision(*ob)) continue;

            const bool aFly = a->IsKnockBacking();
            const bool bFly = b->IsKnockBacking();

            auto pa3 = oa->GetPos();
            auto pb3 = ob->GetPos();

            float dx = pb3.x - pa3.x;
            float dy = pb3.y - pa3.y;

            float distSq = dx * dx + dy * dy;
            float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

            // 法線（A -> B）
            float nx = dx / dist;
            float ny = dy / dist;
            Vector2 n(nx, ny);

            // ---- impact damage（既存）----
            // 飛んでいる敵が、飛んでいない敵に衝突した時だけダメージを渡す
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

            // ---- pushout（既存）----
            float ra = oa->GetCollisionRadius();
            float rb = ob->GetCollisionRadius();

            float overlap = (ra + rb) - dist;
            if (overlap <= 0.0f) continue;

            bool aStop = IsStoppedEnemy(a, oa);
            bool bStop = IsStoppedEnemy(b, ob);

            // =========================================================
            // ✅ ピンボール風：衝突反動（速度の反射 + 伝達）
            //   - Aが飛んでBに突っ込む → Aは反射、Bは押し出されてノックバック
            //   - “横にスッ…”と擦る感じを減らすため、接線成分も少し減衰
            // =========================================================
            {
                // 飛行中の敵のノックバック速度（飛んでなければ0）
                Vector2 va = aFly ? a->GetKnockBackVelocity() : Vector2(0.0f, 0.0f);
                Vector2 vb = bFly ? b->GetKnockBackVelocity() : Vector2(0.0f, 0.0f);

                // 相対速度の法線成分（AがBに向かっているなら +）
                float vrelN = (va - vb).Dot(n);

                // “めり込み方向”の時だけ反動を入れる（離れていく時は不要）
                if (vrelN > 0.0f)
                {
                    // 反発係数（0~1）：大きいほど “팡!”
                    const float e = 0.85f;

                    // 伝達率：Bにどれだけ反動を与えるか
                    const float transfer = 0.80f;

                    // 接線（横滑り）減衰：大きいほど擦り抜けにくい
                    const float dampTangent = 0.20f;

                    // 反動の持続：短く（ピンボールっぽい）
                   // 反動の持続：Bは少し長めに（確実に“飛ぶ”ようにする）
                    const float kickTimeA = 0.18f;
                    const float kickTimeB = 0.35f;

                    // Bが弱くしか動かないのを防ぐための最小反動
                    const float minKickB = 180.0f; // ← ゲームの単位に合わせて 120〜250 で調整

                    auto KillTangent = [&](Vector2 v) -> Vector2
                        {
                            float vn = v.Dot(n);
                            Vector2 vN = vn * n;
                            Vector2 vT = v - vN;
                            vT *= (1.0f - dampTangent);
                            return vN + vT;
                        };

                    // A: 法線方向成分を反射（va' = va - (1+e)*vrelN*n）
                    if (aFly)
                    {
                        // 完全反射速度
                        Vector2 vReflect = va - (1.0f + e) * vrelN * n;

                        // ✅ 反射を弱めて「180度じゃなくて120度くらい」にする
                        // mix=1.0 -> 完全反射(180°寄り)
                        // mix=0.0 -> 反射なし(直進)
                        const float reflectMix = 0.55f; // ★まずは0.55（0.45~0.70で調整）

                        Vector2 vaNew = va * (1.0f - reflectMix) + vReflect * reflectMix;

                        a->SetKnockBackVelocity(KillTangent(vaNew));
                    }

                    // B: 反動を受けて飛ぶ（短時間ノックバック付与）
                    //     ・最低速度を保証して「反動したのに飛ばない」を防ぐ
                    Vector2 impulseB = (1.0f + e) * vrelN * n * transfer;
                    Vector2 kickB = KillTangent(impulseB);

                    // 最小反動保証（小さい衝突でもピンボールっぽく弾く）
                    float len = kickB.Length();
                    if (len < minKickB)
                    {
                        if (len > 0.0001f) kickB *= (minKickB / len);
                        else               kickB = n * minKickB;
                    }

                    b->AddKnockBackImpulse(kickB, kickTimeB);
                }
            }

            // =========================================================
            // ✅ 位置の押し出し（既存）＋ stopDist 固定ロジック維持
            // =========================================================
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

                float ax = -t.x * pushA;
                float ay = -t.y * pushA;
                float bx = t.x * pushB;
                float by = t.y * pushB;

                oa->SetPos(pa3.x + ax, pa3.y + ay, pa3.z);
                ob->SetPos(pb3.x + bx, pb3.y + by, pb3.z);
                continue;
            }

            // 通常の押し出し（Aは -n、Bは +n）
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

                ///BOSS SPAWN COUNT
                if (m_killCount >= 50 && !m_bossSpawned)
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