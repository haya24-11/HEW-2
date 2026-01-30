#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

#ifdef GetObject
#undef GetObject
#endif

#include "GamePlay.h"
#include "Player.h"
#include "Enemy.h"
#include "NormalEnemy.h"
#include "Texture.h"
#include "CameraGlobals.h"
#include <cmath>
#include <cmath>
#include <algorithm> // std::clamp
#include <unordered_map>

static DirectX::SimpleMath::Vector2 ReflectVec(
    const DirectX::SimpleMath::Vector2& v,
    const DirectX::SimpleMath::Vector2& n)
{
    // v' = v - 2*(v·n)*n
    return v - 2.0f * v.Dot(n) * n;
}

static void EnemyReboundBounce(Enemy* a, Object* aObj, Enemy* b, Object* bObj);

static void HeavyPinballHit(Player* playerLogic, Object* playerObj,
    Enemy* enemyLogic, Object* enemyObj);

static void PushOutCircle(Object* playerObj, Object* enemyObj);

static void EnemyReboundTransfer(Enemy* a, Object* aObj, Enemy* b, Object* bObj);

static void EnemyPinballBounce(Enemy* a, Object* aObj, Enemy* b, Object* bObj);


// 強攻撃の同一敵ヒット連打防止用クールダウン
static std::unordered_map<Enemy*, float> s_heavyHitCD;


static float ClampFloat(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void ClampObjectToMap(Object* obj, Object* map)
{
    if (!obj || !map) return;

    auto p = obj->GetPos();
    auto mp = map->GetPos();
    auto ms = map->GetSize();

    float r = obj->GetCollisionRadius();

    float left = mp.x - ms.x * 0.5f + r;
    float right = mp.x + ms.x * 0.5f - r;
    float bottom = mp.y - ms.y * 0.5f + r;
    float top = mp.y + ms.y * 0.5f - r;

    p.x = ClampFloat(p.x, left, right);
    p.y = ClampFloat(p.y, bottom, top);

    obj->SetPos(p.x, p.y, p.z);
}



GamePlay::GamePlay() : Scene(SceneType::GamePlay)
{
}


void GamePlay::InitScene()
{
    // ===== プレイヤー関連テクスチャを事前ロード =====
    PreloadTexture(g_pDevice, "asset/Texture/player_idle.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_walk.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_light.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_heavy.png");
    PreloadTexture(g_pDevice, "asset/Texture/slash_effect.png");

    Object dummy;
    dummy.Init("asset/Texture/player_attack_heavy.png", 1, 1);
    dummy.SetSize(1.0f, 1.0f, 0.0f);
    dummy.SetPos(100000.0f, 100000.0f, 0.0f); // 画面の外
    dummy.Draw();
    dummy.Uninit();

    // カメラの大きさ調整
    m_camera.SetViewSize(1670.0f, 940.0f);
    std::cout << "InitScene" << std::endl;

    // ===== MAP =====
    Object* map = AddObject();
    map->Init("asset/Texture/map.png");
    map->SetPos(0.0f, 0.0f, 0.0f);
    map->SetSize(3000.0f, 3000.0f, 0.0f);

    m_map = map; 


    // ===== Player Logic =====
    m_player = std::make_unique<Player>();

    // ===== Player Object =====
    Object* player = AddObject();
    player->Init("asset/Texture/player_idle.png");
    player->SetSpriteSheet(6, 6);
    player->SetSize(150.0f, 170.0f, 0.0f);  // ★2回入ってたので1つに整理
    player->SetPos(0.0f, 0.0f, 0.0f);
    player->SetCollisionRadius(100.0f);

    m_player->SetObject(player);

    // ===== カメラ初期化（プレイヤー基準） + g_cameraX/Y 同期 =====
    {
        auto p = player->GetPos();
        m_camera.SetPosition({ p.x, p.y });

        // ★重要：初期フレームでズレないように最初から同期しておく
        g_cameraX = p.x;
        g_cameraY = p.y;
    }

    // ===== Enemy Spawner =====
    m_spawner.Init(this, m_player->GetObject());
    m_spawner.RegisterType<NormalEnemy>(1.0f);

    std::cout << "(Debug) GamePlayScene!" << std::endl;

    /////////////////////////////////////////////////////////////////////////////
    // UIのPOS設定はここではなく UpdateUIFollowCamera() で毎フレーム行う
    /////////////////////////////////////////////////////////////////////////////

    // ===== 弱攻撃ボタンUI =====
    LightAttackButton = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)     // ★初期値は適当でOK（後でUpdateUIFollowCameraが上書き）
        ->SetSize(45.0f, 35.0f, 0.0f)
        ->SetAngle(0.0f);
    LightAttackButton->Init("asset/UI/lightattackbutton.png");
    LightAttackButton->SetUI(true);   // ★UI固定

    // ===== 強攻撃ボタンUI =====
    HeavyAttackButton = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(45.0f, 35.0f, 0.0f)
        ->SetAngle(0.0f);
    HeavyAttackButton->Init("asset/UI/heavyattackbutton.png");
    HeavyAttackButton->SetUI(true);

    // ===== プレイヤーHPバー UI =====
    PlayerHeartPointBar = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(420.0f, 300.0f, 0.0f)
        ->SetAngle(0.0f);
    PlayerHeartPointBar->Init("asset/UI/playerheartpointbar.png");
    PlayerHeartPointBar->SetUI(true);
    
    // ===== プレイヤーアイコン UI =====
    PlayerIcon = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(72.0f, 72.0f, 0.0f)
        ->SetAngle(0.0f);
    PlayerIcon->Init("asset/UI/playericon.png");
    PlayerIcon->SetUI(true);

    // ===== バフアイコン =====
    BuffIcons.clear(); // ★2回目開始時に前回のポインタが残らないように一応クリア
    for (int i = 0; i < 5; i++)
    {
        Object* newBuff = AddObject()
            ->SetPos(0.0f, 0.0f, 0.0f)
            ->SetSize(44.0f, 44.0f, 0.0f)
            ->SetAngle(0.0f);

        newBuff->Init("asset/UI/bufficon.png");
        newBuff->SetUI(true);

        BuffIcons.push_back(newBuff);
    }

    // ===== 魔法陣 UI =====
    MagicCircle = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(360.0f, 360.0f, 0.0f)
        ->SetAngle(0.0f)
        ->SetColor(1, 1, 1, 0.55f);
    MagicCircle->Init("asset/UI/magiccircle.png");
    MagicCircle->SetUI(true);

    // ===== 経験値バー UI 関連 =====
    // 経験値バー 背景
    ExpBarBack = AddObject()
        ->SetPos(0.0f, 1000.0f, 0.0f)
        ->SetSize(1780.0f, 295.0f, 0.0f)   // ※見やすい太さ（必要なら調整）
        ->SetAngle(0.0f);
    ExpBarBack->Init("asset/UI/expbar_back.png"); // ここは実際のパスに合わせて
    ExpBarBack->SetUI(true);

    // 経験値バー ゲージ ※todo
    ExpBarGauge = AddObject()
        ->SetPos(0.0f, 1000.0f, 0.0f)
        ->SetSize(20.0f, 30.0f, 0.0f)   // ※見やすい太さ（必要なら調整）
        ->SetAngle(0.0f);
    ExpBarGauge->Init("asset/UI/expbar_gauge.png"); // ここは実際のパスに合わせて
    ExpBarGauge->SetUI(true);

    // 経験値バー フレーム
    ExpBarFrame = AddObject()
        ->SetPos(0.0f, 1000.0f, 0.0f)
        ->SetSize(1780.0f, 300.0f, 0.0f)   // ※見やすい太さ（必要なら調整）
        ->SetAngle(0.0f);
    ExpBarFrame->Init("asset/UI/expbar_frame.png"); // ここは実際のパスに合わせて
    ExpBarFrame->SetUI(true);


    // ★重要：最初のフレームからUI位置を確定（2回目開始のズレ防止）
    UpdateUIFollowCamera();
}


void GamePlay::UpdateScene(float deltaTime)
{
    std::cout << "objects: " << objects.size() << std::endl;

    if (!m_player) return;
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    Object* playerObj = m_player->GetObject();
    if (!playerObj) return;

    const auto oldPos = playerObj->GetPos();

    m_player->Update(deltaTime);

    if (m_player->IsAttackInputTriggered())
    {
        m_attackEffects.push_back(
            new AttackSlashEffect(this,
                m_player->GetObject(),
                m_player->IsFacingRight())
        );
    }

    /* ================================
   ★ 攻撃エフェクトの更新＆破棄
   ================================ */
    for (auto it = m_attackEffects.begin(); it != m_attackEffects.end(); )
    {
        AttackSlashEffect* e = *it;

        if (!e)
        {
            it = m_attackEffects.erase(it);
            continue;
        }

        e->Update(deltaTime);

        if (e->IsDead())
        {
            e->Uninit();
            delete e;
            it = m_attackEffects.erase(it);
        }
        else
        {
            ++it;
        }
    }

    m_spawner.Update(deltaTime);

    // 強攻撃ヒットCD更新
    for (auto it = s_heavyHitCD.begin(); it != s_heavyHitCD.end(); )
    {
        it->second -= deltaTime;
        if (it->second <= 0.0f) it = s_heavyHitCD.erase(it);
        else ++it;
    }

    m_rotation += m_rotationSpeed * deltaTime;
    MagicCircle->SetAngle(m_rotation);

    // 360度超えたら戻す（任意だけどおすすめ）
    if (m_rotation >= 360.0f)
        m_rotation -= 360.0f;

    for (int iter = 0; iter < 3; ++iter)
    {
        bool pushed = false;

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;

            Object* enemyObj = e->GetObject();
            if (!enemyObj) continue;

            if (playerObj->CheckCollision(*enemyObj))
            {
                if (m_player->IsHeavyDashing())
                {
                    HeavyPinballHit(m_player.get(), playerObj, e.get(), enemyObj);
                }
                else
                {
                    // ✅ 敵が吹き飛び中ならプレイヤーを押し返さない
                    if (!e->IsKnockBacking())
                    {
                        PushOutCircle(playerObj, enemyObj);
                        pushed = true;
                    }
                    else
                    {
                        //（任意）めり込みだけ軽く解消：敵の速度は維持
                        auto p = playerObj->GetPos();
                        auto q = enemyObj->GetPos();
                        float dx = q.x - p.x;
                        float dy = q.y - p.y;
                        float d2 = dx * dx + dy * dy;
                        if (d2 > 0.0001f)
                        {
                            float d = sqrtf(d2);
                            dx /= d; dy /= d;
                            enemyObj->SetPos(q.x + dx * 3.0f, q.y + dy * 3.0f, q.z);
                            pushed = true;
                        }
                    }
                }
            }
        }

        if (!pushed) break;
    }

    //PAD処理シ－ン切り替え
    static WORD prevButtons = 0;

    XINPUT_STATE pad{};
    WORD buttons = 0;
    if (XInputGetState(0, &pad) == ERROR_SUCCESS)
        buttons = pad.Gamepad.wButtons;

    auto PadTrigger = [&](WORD mask) -> bool
        {
            return (buttons & mask) && !(prevButtons & mask);
        };

    if (Input::GetKeyTrigger(VK_SPACE) || PadTrigger(XINPUT_GAMEPAD_RIGHT_SHOULDER))
    {
        SetNextScene(SceneType::Result);
    }

    // ✅ すべての移動/衝突処理が終わった「最後」にカメラを確定する
    {
        auto p = playerObj->GetPos();
        float camX = p.x, camY = p.y;

        if (m_map)
        {
            auto mp = m_map->GetPos();
            auto ms = m_map->GetSize();

            float left = mp.x - ms.x * 0.5f;
            float right = mp.x + ms.x * 0.5f;
            float bottom = mp.y - ms.y * 0.5f;
            float top = mp.y + ms.y * 0.5f;

            float halfVW = m_camera.GetViewW() * 0.5f;
            float halfVH = m_camera.GetViewH() * 0.5f;

            float loX = left + halfVW;
            float hiX = right - halfVW;
            float loY = bottom + halfVH;
            float hiY = top - halfVH;

            // ✅ lo > hi の場合（ビューがマップより大きい等）は中心固定
            if (loX > hiX) camX = (left + right) * 0.5f;
            else           camX = ClampFloat(camX, loX, hiX);

            if (loY > hiY) camY = (bottom + top) * 0.5f;
            else           camY = ClampFloat(camY, loY, hiY);
        }

        m_camera.SetPosition({ camX, camY });
        g_cameraX = camX;
        g_cameraY = camY;
    }
    // ===== プレイヤーをマップ外に出さない =====
   /* if (m_map && playerObj)
    {
        auto p = playerObj->GetPos();
        auto mp = m_map->GetPos();
        auto ms = m_map->GetSize();

        float r = playerObj->GetCollisionRadius();

        float left = mp.x - ms.x * 0.5f + r;
        float right = mp.x + ms.x * 0.5f - r;
        float bottom = mp.y - ms.y * 0.5f + r;
        float top = mp.y + ms.y * 0.5f - r;

        p.x = ClampFloat(p.x, left, right);
        p.y = ClampFloat(p.y, bottom, top);

        playerObj->SetPos(p.x, p.y, p.z);
    }*/

    {
        static bool s_bossHasSpawned = false; // ✅ ボスが一度でも出現したか
        bool bossFound = false;
        bool bossAlive = false;

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;

            if (e->IsBoss())
            {
                bossFound = true;
                s_bossHasSpawned = true;

                if (e->IsAlive())
                    bossAlive = true;

                break; // ボスは1体想定
            }
        }

        // ✅ 出現済みなのに、今フレームはボスがいない/死んでいる → Resultへ
        // bossFound==false は「死んで Cleanup で消えた後」も含む
        if (s_bossHasSpawned && (!bossFound || !bossAlive))
        {
            SetNextScene(SceneType::Result);
            return;
        }
    }

    if (m_map)
    {
        ClampObjectToMap(playerObj, m_map);

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;
            if (auto* eo = e->GetObject())
                ClampObjectToMap(eo, m_map);
        }
    }



    prevButtons = buttons;
    UpdateUIFollowCamera();
}

void GamePlay::DrawScene()
{
    for (auto& obj : objects)
    {
        int frame = -1;

        if (obj.get() == m_player->GetObject())
        {
            frame = m_player->GetAnimFrame();
        }
        else
        {
            for (const auto& e : m_spawner.GetEnemies())
            {
                if (!e) continue;
                if (e->GetObject() == obj.get())
                {
                    frame = e->GetAnimFrame();
                    break;
                }
            }
        }

        if (frame >= 0) obj->Draw(frame);
        else            obj->Draw();
    }

}



void GamePlay::UninitScene()
{
    Object::ReleaseTextureCache();
    std::cout << "UninitScene" << std::endl;
    for (auto e : m_attackEffects)
    {
        delete e;
    }
    m_attackEffects.clear();
}

static void PushOutCircle(Object* playerObj, Object* enemyObj)
{
    if (!playerObj || !enemyObj) return;
    if (!playerObj->CheckCollision(*enemyObj)) return;

    auto p3 = playerObj->GetPos();
    auto e3 = enemyObj->GetPos();

    float dx = p3.x - e3.x;
    float dy = p3.y - e3.y;

    float distSq = dx * dx + dy * dy;

    // 実際の当たり判定半径を使用（Object に GetCollisionRadius() が必要）
    float rp = playerObj->GetCollisionRadius();
    float re = enemyObj->GetCollisionRadius();

    // 距離がほぼ0のとき（法線が作れず押し出しが暴れるのを防ぐ）
    float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.0f;
    if (dist <= 0.0001f)
    {
        // 完全に重なっている場合：適当な方向に少しだけずらして抜け出す（瞬間移動っぽさ防止）
        playerObj->SetPos(p3.x + 1.0f, p3.y, p3.z);
        return;
    }

    // 重なり量（半径合計 - 実距離）
    float overlap = (rp + re) - dist;
    if (overlap <= 0.0f) return;

    // 押し出し方向（正規化ベクトル）
    float nx = dx / dist;
    float ny = dy / dist;

    // 1フレームで押し出す最大量を制限（大きく飛ぶ＝瞬間移動を防ぐ）
    const float maxPushPerFrame = 10.0f; 
    float push = overlap;
    if (push > maxPushPerFrame) push = maxPushPerFrame;

    // プレイヤーだけを押し出す（敵は固定）
    playerObj->SetPos(p3.x + nx * push, p3.y + ny * push, p3.z);
}

void GamePlay::UpdateUIFollowCamera()
{
    const float halfW = SCREEN_WIDTH * 0.5f;
    const float halfH = SCREEN_HEIGHT * 0.5f;
    const float pad = 30.0f;

    // 画面基準の左上付近
    const float hpBarX = -halfW + pad + 180.0f;
    const float hpBarY = halfH - pad - 44.0f;

    if (PlayerHeartPointBar)
        PlayerHeartPointBar->SetPos(hpBarX, hpBarY-20, 0.0f);

    // アイコン
    const float hpW = 360.0f;
    const float iconSize = 72.0f;
    const float circlePadding = 22.0f;

    const float circleCenterX = hpBarX - (hpW * 0.5f) + (iconSize * 0.5f) + circlePadding;
    const float circleCenterY = hpBarY + 10.0f;


    if (PlayerIcon)
    {
        PlayerIcon->SetPos(circleCenterX+0, circleCenterY-10, 0.0f);
        PlayerIcon->SetSize(iconSize, iconSize, 0.0f);
    }

    // バフアイコン（HPバーの下）
    for (int i = 0; i < (int)BuffIcons.size(); i++)
    {
        if (!BuffIcons[i]) continue;

        BuffIcons[i]->SetPos(
            -halfW + pad + 155.0f + (i * 48.0f),
            halfH - pad - 36.0f - 100.0f,
            0.0f
        );
    }

    // 右下のボタン
    if (LightAttackButton)
        LightAttackButton->SetPos(halfW - pad - 70.0f, -halfH + pad + 30.0f, 0.0f);

    if (HeavyAttackButton)
        HeavyAttackButton->SetPos(halfW - pad - 160.0f, -halfH + pad + 30.0f, 0.0f);

    // 右上の魔法陣
    if (MagicCircle)
        MagicCircle->SetPos(halfW - pad - 10.0f, halfH - pad - 10.0f, 0.0f);

    // 経験値バー（HPバー付近）
    if (ExpBarFrame)
    {
        const float gapY = -855.0f;
        ExpBarBack->SetPos(hpBarX +665.0f, hpBarY + gapY, 0.0f);     // 経験値バー 背景
        ExpBarGauge->SetPos(hpBarX + -180.0f, hpBarY + gapY, 0.0f); // 経験値バー ゲージ ※todo
        ExpBarFrame->SetPos(hpBarX + 665.0f, hpBarY + gapY, 0.0f);  // 経験値バー フレーム
    }
}


static void HeavyPinballHit(Player* playerLogic, Object* playerObj,
    Enemy* enemyLogic, Object* enemyObj)
{
    if (!playerLogic || !playerObj || !enemyLogic || !enemyObj) return;

    // すでにノックバック中なら無視（連打防止）
    if (enemyLogic->IsKnockBacking()) return;

    // 同じ敵へのクールタイム（連打防止）
    const float hitCooldown = 0.20f;
    if (s_heavyHitCD.find(enemyLogic) != s_heavyHitCD.end()) return;

    auto p3 = playerObj->GetPos();
    auto e3 = enemyObj->GetPos();

    DirectX::SimpleMath::Vector2 p(p3.x, p3.y);
    DirectX::SimpleMath::Vector2 e(e3.x, e3.y);

    // 衝突法線（プレイヤー → 敵）
    auto n = e - p;
    if (n.LengthSquared() < 0.0001f)
        n = playerLogic->GetHeavyDashVelocity();
    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    auto vP = playerLogic->GetHeavyDashVelocity();
    float relN = vP.Dot(n);
    if (relN < 50.0f) return;

    // =========================
    // ① ダメージ（power × 強攻撃倍率 + 速度ボーナス）
    // =========================
    int baseDmg = (int)(playerLogic->GetPower() * playerLogic->GetHeavyMul());
    int bonus = (int)(relN / 120.0f);
    int dmg = baseDmg + bonus;
    if (dmg < 1) dmg = 1;

    // =========================
    // ② ノックバック
    // =========================
    const float baseKick = 100.0f;
    const float kickBySpeed = 0.7f;
    const float maxKick = 300.0f;

    float kick = baseKick + relN * kickBySpeed;
    if (kick > maxKick) kick = maxKick;

    // ✅ ここ！強攻撃で当たった時だけ「止まった後に死ぬ」モードON
    enemyLogic->EnableDeathAfterKnockback(true);

    // ✅ 吹き飛んでいく敵が持ち運ぶ衝突ダメージを保存
    enemyLogic->SetImpactDamage(dmg);

    // ✅ ダメージは1回だけ
    enemyLogic->TakeDamage(dmg);

    // ✅ 吹き飛ばす
    enemyLogic->KnockBack(n * kick);

    // クールタイム開始
    s_heavyHitCD[enemyLogic] = hitCooldown;
}




static void EnemyReboundTransfer(Enemy* a, Object* aObj, Enemy* b, Object* bObj)
{
    if (!a || !b || !aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();

    // 両方とも飛んでいる / 両方とも歩行中 なら処理しない
    if (aFly == bFly) return;

    Enemy* fly = aFly ? a : b;       // 飛んでいた敵
    Enemy* walk = aFly ? b : a;      // 歩行していた敵（これから飛ばされる側）
    Object* flyOb = aFly ? aObj : bObj;
    Object* walkOb = aFly ? bObj : aObj;

    auto v = fly->GetKnockBackVelocity();
    float speed = v.Length();
    if (speed < 1.0f)
    {
        // ほぼ止まっているならノックバック終了
        fly->StopKnockBack();
        return;
    }

    // 衝突方向（飛んでいた敵 -> 歩いていた敵）
    auto fp = flyOb->GetPos();
    auto wp = walkOb->GetPos();
    DirectX::SimpleMath::Vector2 n(wp.x - fp.x, wp.y - fp.y);

    // 完全に重なっている場合は速度方向を使って法線を作る
    if (n.LengthSquared() < 0.0001f) n = v;
    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    // fly が実際に walk 側へ押し込んでいる時だけ処理（すれ違い/背中側ヒットは無視）
    if (v.Dot(n) <= 0.0f) return;

    // ===== 反動の強さ（調整ポイント） =====
    const float reboundRate = 0.85f; // 0～1：小さいほど反動が弱い
    const float minKick = 250.0f;    // 最低反動速度
    const float maxKick = 1600.0f;   // 最大反動速度

    float kick = speed * reboundRate;
    if (kick < minKick) kick = minKick;
    if (kick > maxKick) kick = maxKick;

    // 1) 飛んでいた敵は止める
    fly->StopKnockBack();

    // 2) 歩いていた敵に反動を渡して飛ばす
    walk->KnockBack(n * kick);

    // 重なり解消（次フレームで連続衝突しないように）：walk だけ少し外へ出す
    float rf = flyOb->GetCollisionRadius();
    float rw = walkOb->GetCollisionRadius();

    float dx = wp.x - fp.x;
    float dy = wp.y - fp.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.0001f) dist = 0.0001f;

    float overlap = (rf + rw) - dist;
    if (overlap > 0.0f)
    {
        float push = overlap + 2.0f; // +2 は再衝突防止の余裕
        walkOb->SetPos(wp.x + n.x * push, wp.y + n.y * push, wp.z);
    }
}

static void EnemyReboundBounce(Enemy* a, Object* aObj, Enemy* b, Object* bObj)
{
    if (!a || !b || !aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();
    if (!aFly && !bFly) return; // 両方とも飛んでいなければ無視

    auto a3 = aObj->GetPos();
    auto b3 = bObj->GetPos();
    DirectX::SimpleMath::Vector2 pa(a3.x, a3.y);
    DirectX::SimpleMath::Vector2 pb(b3.x, b3.y);

    // 衝突法線（a -> b）
    DirectX::SimpleMath::Vector2 n = pb - pa;
    if (n.LengthSquared() < 0.0001f)
    {
        // 完全に重なっているなら相対速度っぽいものから法線を作る
        DirectX::SimpleMath::Vector2 rel(0, 0);
        if (aFly) rel += a->GetKnockBackVelocity();
        if (bFly) rel -= b->GetKnockBackVelocity();
        n = (rel.LengthSquared() > 0.0001f) ? rel : DirectX::SimpleMath::Vector2(1, 0);
    }
    n.Normalize();

    // ===== チューニングポイント =====
    const float restitution = 0.85f; // 1.0に近いほど“よく跳ねる”（0.7～0.9推奨）
    const float minSpeed = 150.0f;   // 反射後の最低速度
    const float maxSpeed = 1600.0f;  // 反射後の最大速度

    auto bounceOne = [&](Enemy* e, const DirectX::SimpleMath::Vector2& normal)
        {
            auto v = e->GetKnockBackVelocity();

            // normal 方向へ食い込んでいる時だけ反射（くっついた状態での連打を防ぐ）
            if (v.Dot(normal) <= 0.0f) return;

            // 反射：v' = v - 2*(v·n)*n
            auto vNew = ReflectVec(v, normal) * restitution;

            // 速度の下限/上限をクランプ
            float s = vNew.Length();
            if (s < minSpeed) vNew = (-normal) * minSpeed;
            if (s > maxSpeed) vNew *= (maxSpeed / s);

            // ✅ タイマーは維持して速度だけ更新（ノックバック継続）
            e->SetKnockBackVelocity(vNew);
        };

    if (aFly) bounceOne(a, n);    // a は +n を基準に反射
    if (bFly) bounceOne(b, -n);   // b は -n を基準に反射

    // ===== 重なり解消（挟まって震えるの防止） =====
    float ra = aObj->GetCollisionRadius();
    float rb = bObj->GetCollisionRadius();

    DirectX::SimpleMath::Vector2 d = pb - pa;
    float dist = d.Length();
    if (dist < 0.0001f) dist = 0.0001f;

    float overlap = (ra + rb) - dist;
    if (overlap > 0.0f)
    {
        // お互い半分ずつ離して分離
        float push = overlap * 0.5f + 1.0f;
        aObj->SetPos(a3.x - n.x * push, a3.y - n.y * push, a3.z);
        bObj->SetPos(b3.x + n.x * push, b3.y + n.y * push, b3.z);
    }
}



static float ClampF(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}
static void EnemyPinballBounce(Enemy* a, Object* aObj, Enemy* b, Object* bObj)
{
    // nullチェック
    if (!a || !b || !aObj || !bObj) return;

    // 当たっていなければ何もしない
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();

    // 両方とも歩き状態なら無視（ピンボール処理の対象外）
    if (!aFly && !bFly) return;

    // 位置取得（3D -> 2Dへ）
    auto ap3 = aObj->GetPos();
    auto bp3 = bObj->GetPos();
    DirectX::SimpleMath::Vector2 ap(ap3.x, ap3.y);
    DirectX::SimpleMath::Vector2 bp(bp3.x, bp3.y);

    // 法線（a -> b 方向）
    DirectX::SimpleMath::Vector2 n = bp - ap;
    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    // ==== チューニングポイント（体感が変わる）====
    const float transferRate = 0.9f; // ぶつかった相手に速度を渡す比率（0～1）
    const float bounceRate = 0.8f; // 反射時の減衰（0～1）
    const float minKick = 600.0f; // 相手に与える最小速度
    const float maxKick = 1400.0f; // 相手に与える最大速度
    const float stopSpeed = 15.0f;   // これ未満なら停止扱い（任意）

    // 重なり解消（連続衝突で暴れないように分離する）
    auto ResolveOverlap = [&](Object* o1, Object* o2, const DirectX::SimpleMath::Vector2& normal)
        {
            float r1 = o1->GetCollisionRadius();
            float r2 = o2->GetCollisionRadius();

            auto p1 = o1->GetPos();
            auto p2 = o2->GetPos();

            float dx = (p2.x - p1.x);
            float dy = (p2.y - p1.y);
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist < 0.0001f) dist = 0.0001f;

            float overlap = (r1 + r2) - dist;
            if (overlap > 0.0f)
            {
                float push = overlap + 2.0f; // +2 は再衝突防止の余裕
                // 両方を半分ずつ押し戻して分離（連打衝突防止に重要）
                o1->SetPos(p1.x - normal.x * (push * 0.5f), p1.y - normal.y * (push * 0.5f), p1.z);
                o2->SetPos(p2.x + normal.x * (push * 0.5f), p2.y + normal.y * (push * 0.5f), p2.z);
            }
        };

    // 「飛んでいる敵(fly)」が「相手(other)」に当たったときの処理
    auto BounceOne = [&](Enemy* fly, Object* flyObj, Enemy* other, Object* otherObj,
        DirectX::SimpleMath::Vector2 normalFlyToOther)
        {
            auto v = fly->GetKnockBackVelocity();
            float vn = v.Dot(normalFlyToOther);

            // 相手方向へ実際に食い込んでいる時だけ処理（離れていく/かすりは無視）
            if (vn <= 0.0f) return;

            // 1) 相手に速度を渡してノックバックさせる
            float kick = ClampF(vn * transferRate, minKick, maxKick);
            other->KnockBack(normalFlyToOther * kick);

            // 2) 飛んでいる側は反射（鏡面反射: v - 2*(v·n)*n）して減衰
            DirectX::SimpleMath::Vector2 vRef = v - 2.0f * vn * normalFlyToOther;
            vRef *= bounceRate;

            if (vRef.Length() < stopSpeed)
            {
                // 遅すぎるなら停止（任意）
                fly->StopKnockBack();
            }
            else
            {
                // 止めずに速度だけ差し替えて飛び続けさせる
                fly->SetKnockBackVelocity(vRef);
            }

            // 3) 重なり解消（これがないと毎フレーム当たり続けて暴走しやすい）
            ResolveOverlap(flyObj, otherObj, normalFlyToOther);
        };

    if (aFly && !bFly)
    {
        // a が飛んでいて b が歩き → a から b へ
        BounceOne(a, aObj, b, bObj, n);
    }
    else if (!aFly && bFly)
    {
        // b が飛んでいて a が歩き → b から a へ（法線は逆）
        BounceOne(b, bObj, a, aObj, -n);
    }
    else
    {
        // 両方飛んでいる → 簡易的にお互い反射させる
        //（必要なら「弾性衝突」っぽく改善可能）
        auto va = a->GetKnockBackVelocity();
        auto vb = b->GetKnockBackVelocity();

        float vna = va.Dot(n);
        float vnb = vb.Dot(n);

        // お互い近づいているときだけ
        if ((vna - vnb) <= 0.0f) return;

        DirectX::SimpleMath::Vector2 vaRef = va - 2.0f * vna * n;
        DirectX::SimpleMath::Vector2 vbRef = vb - 2.0f * vnb * (-n);

        vaRef *= bounceRate;
        vbRef *= bounceRate;

        a->SetKnockBackVelocity(vaRef);
        b->SetKnockBackVelocity(vbRef);

        ResolveOverlap(aObj, bObj, n);
    }
}
