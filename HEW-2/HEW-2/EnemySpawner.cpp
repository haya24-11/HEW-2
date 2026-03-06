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

    m_killCount = 0;
    m_bossSpawned = false;

    // ★念のため同期
    m_stopLock.assign(m_enemies.size(), 0);
}

void EnemySpawner::Update(float deltaTime)
{
    if (!m_scene || !m_player) return;
    if (m_entries.empty()) return;

    // =========================
    // (1) 既存の敵を毎フレーム更新
    // =========================
    for (auto& e : m_enemies)
    {
        if (e) e->Update(deltaTime);
    }

    //衝突エフェクト更新
    UpdateImpactEffects(deltaTime);

    // =========================
    // (1.2) 死亡した敵を掃除（aliveCountも更新）
    // =========================
    CleanupDeadEnemies();

    // =========================
    // (1.5) 敵同士の当たり判定（押し出し）
    // =========================
    for (int iter = 0; iter < 3; ++iter)
    {
        ResolveEnemyCollisions();
    }

    // =========================
    // (2) タイプ別スポーン（interval / maxAlive / timer / aliveCount）
    // =========================
    for (int i = 0; i < (int)m_entries.size(); ++i)
    {
        auto& ent = m_entries[i];

        // 日本語コメント：タイプ別タイマー進行
        ent.timer += deltaTime;

        // 日本語コメント：このタイプの最大数に達しているならスキップ
        if (ent.aliveCount >= ent.maxAlive)
            continue;

        // 日本語コメント：まだインターバル未満ならスキップ
        if (ent.timer < ent.interval)
            continue;

        // 日本語コメント：このタイプをスポーンする
        std::unique_ptr<Enemy> enemy = ent.createFn ? ent.createFn() : nullptr;
        if (!enemy) continue;

        Enemy::SpawnConfig cfg = enemy->GetSpawnConfig();

        Object* obj = m_scene->AddObject();
        if (!obj) continue;

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

        enemy->SetObject(obj);
        enemy->SetTarget(m_player);

        enemy->SetGamePlay(static_cast<GamePlay*>(m_scene));

        enemy->SetDeathDelay(cfg.dieDelay);
        enemy->SetDisappearDelay(cfg.disappearDelay);

        enemy->OnSpawned();

        enemy->SetChaseEnabled(true);

        if (cfg.stopDist > 0.0f) enemy->SetChaseStopDistance(cfg.stopDist);
        else                    enemy->SetChaseStopDistance(0.0f);

        // ✅重要：この敵がどのタイプ（Entry）から出たかを覚えさせる
        enemy->SetSpawnerEntryIndex(i);

        // ✅重要：タイプ別の生存数を増やす
        ent.aliveCount++;

        m_enemies.emplace_back(std::move(enemy));

        // ✅スポーンしたのでタイプ別タイマーリセット
        ent.timer = 0.0f;

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
    if (!m_player || !m_playerLogic) return;

    using namespace DirectX::SimpleMath;

    auto pp3 = m_player->GetPos();
    Vector2 playerPos(pp3.x, pp3.y);

    auto IsStoppedEnemy = [&](Enemy* e, Object* eo) -> bool
        {
            if (!e || (size_t)e < 0x10000 || !eo || (size_t)eo < 0x10000) return false;
            const float stop = e->GetChaseStopDistance();
            if (stop <= 0.0f) return false;
            if (e->IsKnockBacking()) return false;
            auto p3 = eo->GetPos();
            Vector2 pos(p3.x, p3.y);
            float dist = (pos - playerPos).Length();
            return dist <= (stop + 2.0f);
        };

    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        Enemy* a = m_enemies[i].get();
        if (!a || (size_t)a < 0x10000 || a == (void*)0xFFFFFFFFFFFFFFFF) continue;
        if (!a->IsAlive()) continue;

        Object* oa = a->GetObject();
        if (!oa || (size_t)oa < 0x10000) continue;

        for (size_t j = i + 1; j < m_enemies.size(); ++j)
        {
            Enemy* b = m_enemies[j].get();
            if (!b || (size_t)b < 0x10000 || b == (void*)0xFFFFFFFFFFFFFFFF) continue;
            if (!b->IsAlive()) continue;

            Object* ob = b->GetObject();
            if (!ob || (size_t)ob < 0x10000) continue;

            try {
                if (!oa->CheckCollision(*ob)) continue;
            }
            catch (...) { continue; }

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

            // 🔴 プレイヤーから強化済みの「強攻撃ノックバック力」を取得
            float playerKBPower = m_playerLogic->GetHeavyKnockBackPower();

            // ---- impact damage ----
            if (aFly && !bFly) {
                int impact = 0;
                if (a->TryConsumeImpactDamage(impact)) {
                    b->TakeDamage(impact);
                    // 🔴 スキルで上がったパワーを連鎖先の敵(b)にも一部伝える
                    b->AddKnockBackImpulse(n * (playerKBPower * 0.4f), 0.35f);

                    SpawnImpactEffect((pa3.x + pb3.x) * 0.5f, (pa3.y + pb3.y) * 0.5f);
                    if (a->GetGamePlay()) a->GetGamePlay()->GetCombo().AddHit();
                }
            }
            else if (!aFly && bFly) {
                int impact = 0;
                if (b->TryConsumeImpactDamage(impact)) {
                    a->TakeDamage(impact);
                    // 🔴 スキルで上がったパワーを連鎖先の敵(a)にも一部伝える
                    a->AddKnockBackImpulse(-n * (playerKBPower * 0.4f), 0.35f);

                    SpawnImpactEffect((pa3.x + pb3.x) * 0.5f, (pa3.y + pb3.y) * 0.5f);
                    if (b->GetGamePlay()) b->GetGamePlay()->GetCombo().AddHit();
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
                if (vrelN > 0.0f) {
                    const float e = 0.85f;
                    const float transfer = 0.80f;
                    const float dampTangent = 0.20f;
                    const float kickTimeB = 0.35f;

                    // 🔴 反射時の最低速度もプレイヤーの強化値に合わせて底上げ（止まりにくくする）
                    const float minKickB = 180.0f + (playerKBPower * 0.1f);

                    auto KillTangent = [&](Vector2 v) -> Vector2 {
                        float vn = v.Dot(n);
                        Vector2 vN = vn * n;
                        Vector2 vT = v - vN;
                        vT *= (1.0f - dampTangent);
                        return vN + vT;
                        };
                    if (aFly) {
                        Vector2 vReflect = va - (1.0f + e) * vrelN * n;
                        const float reflectMix = 0.55f;
                        Vector2 vaNew = va * (1.0f - reflectMix) + vReflect * reflectMix;
                        a->SetKnockBackVelocity(KillTangent(vaNew));
                    }
                    Vector2 impulseB = (1.0f + e) * vrelN * n * transfer;
                    Vector2 kickB = KillTangent(impulseB);
                    float len = kickB.Length();
                    if (len < minKickB) kickB = (len > 0.0001f) ? kickB * (minKickB / len) : n * minKickB;
                    b->AddKnockBackImpulse(kickB, kickTimeB);
                }
            }

            float pushA = overlap * 0.5f, pushB = overlap * 0.5f;
            if (aStop && !bStop) { pushA = 0.0f; pushB = overlap; }
            else if (!aStop && bStop) { pushA = overlap; pushB = 0.0f; }
            else if (aStop && bStop) {
                Vector2 posA(pa3.x, pa3.y), posB(pb3.x, pb3.y);
                auto r = (posA + posB) * 0.5f - playerPos;
                float rlen = r.Length();
                r = (rlen < 0.0001f) ? Vector2(1, 0) : r / rlen;
                Vector2 t(-r.y, r.x);
                if ((posB - posA).Dot(t) < 0.0f) t = -t;
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
        // 🔴 ヌルチェック
        if (!e || (size_t)e < 0x10000) {
            it = m_enemies.erase(it);
            continue;
        }

        if (!e->IsAlive())
        {
            // タイプ別 aliveCount を減らす
            const int idx = e->GetSpawnerEntryIndex();
            if (0 <= idx && idx < (int)m_entries.size()) {
                if (m_entries[idx].aliveCount > 0) m_entries[idx].aliveCount--;
            }

            // EXP付与
            if (!e->IsRewardGiven()) {
                if (m_playerLogic) m_playerLogic->AddExp(e->GetExpValue());
                e->MarkRewardGiven();
            }

            // ボス出現判定
            if (!e->IsBoss()) {
                m_killCount++;
                if (m_killCount >= 50 && !m_bossSpawned) SpawnBoss();
            }

            // 🔴 削除順序の安全化：Objectへの参照を断ってから消す
            if (Object* o = e->GetObject()) {
                e->SetObject(nullptr); // Enemy側のポインタを先に消す
                if (m_scene) m_scene->RemoveObject(o);
            }

            it = m_enemies.erase(it);
            continue;
        }
        ++it;
    }
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

    // 日本語コメント：ボスはタイプ別制限に入れない（EntryIndexは未設定のままでOK）
    // boss->SetSpawnerEntryIndex(...); // 必要なら設定しても良い

    m_enemies.emplace_back(std::move(boss));
    m_bossSpawned = true;

    // ★同期（保険）
    m_stopLock.assign(m_enemies.size(), 0);
}

// =========================
// Impact Effect (Enemy vs Enemy 衝突)
// =========================
void EnemySpawner::SpawnImpactEffect(float x, float y)
{
    if (!m_scene) return;

    //エフェクト用オブジェクト生成
    Object* o = m_scene->AddObject();
    if (!o) return;

    // ★スプライトシート設定（ComboEffect.pngが 5x2(計10フレーム) 想定）
    const int sheetX = 5;
    const int sheetY = 2;

    o->Init("asset/Texture/ComboEffect.png", sheetX, sheetY);
    o->SetSpriteSheet(sheetX, sheetY);

    //サイズは好みで調整
    o->SetSize(180.0f, 180.0f, 0.0f);
    o->SetCollisionRadius(0.0f);

    //衝突地点に出す
    o->SetPos(x, y, 0.0f);

    //最初のフレーム
    o->SetAnimFrame(10);

    ImpactFX fx;
    fx.obj = o;
    fx.t = 0.0f;
    fx.frame = 0;
    m_impactFX.push_back(fx);
}

void EnemySpawner::UpdateImpactEffects(float dt)
{
    // 日本語コメント：10フレームを一定間隔で進めて消す
    const float frameTime = 0.18f;  // 1フレーム時間
    const int totalFrames = 10;     // 5x2 = 10

    for (auto it = m_impactFX.begin(); it != m_impactFX.end(); )
    {
        if (!it->obj)
        {
            it = m_impactFX.erase(it);
            continue;
        }

        it->t += dt;

        while (it->t >= frameTime)
        {
            it->t -= frameTime;
            it->frame++;

            if (it->frame < totalFrames)
            {
                it->obj->SetAnimFrame(it->frame);
            }
            else
            {
                // 日本語コメント：終了 → Sceneから削除
                if (m_scene) m_scene->RemoveObject(it->obj);
                it = m_impactFX.erase(it);
                goto NEXT;
            }
        }

        ++it;
    NEXT:
        ;
    }
}