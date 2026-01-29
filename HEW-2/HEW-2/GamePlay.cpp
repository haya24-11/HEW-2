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
#include <algorithm> // (std::clamp 쓰고 싶으면)
#include <cmath>

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

    Object dummy;
    dummy.Init("asset/Texture/player_attack_heavy.png", 1, 1);
    dummy.SetSize(1.0f, 1.0f, 0.0f);
    dummy.SetPos(100000.0f, 100000.0f, 0.0f); // 画面の外
    dummy.Draw();
    dummy.Uninit();

    // カメラの大きさ調整
    m_camera.SetViewSize(640.0f, 320.0f);
    std::cout << "InitScene" << std::endl;

    // ===== MAP =====
    Object* map = AddObject();
    map->Init("asset/Texture/map.png");
    map->SetPos(0.0f, 0.0f, 0.0f);
    map->SetSize(3000.0f, 3000.0f, 0.0f);

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
        ->SetSize(420.0f, 240.0f, 0.0f)
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
        ->SetSize(180.0f, 180.0f, 0.0f)
        ->SetAngle(0.0f)
        ->SetColor(1, 1, 1, 0.55f);
    MagicCircle->Init("asset/UI/magiccircle.png");
    MagicCircle->SetUI(true);

    // ===== 経験値バー UI =====
    ExpBar = AddObject()
        ->SetPos(0.0f, 0.0f, 0.0f)
        ->SetSize(280.0f, 100.0f, 0.0f)   // ※見やすい太さ（必要なら調整）
        ->SetAngle(0.0f);
    ExpBar->Init("asset/UI/expbar.png"); // ここは実際のパスに合わせて
    ExpBar->SetUI(true);

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

    {
        auto p = playerObj->GetPos();
        m_camera.SetPosition({ p.x, p.y });
    }

    m_spawner.Update(deltaTime);
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
                    // ✅ 적이 날아가는 중이면 플레이어를 밀지 말기 (멈춘 것처럼 보이는 원인 제거)
                    if (!e->IsKnockBacking())
                    {
                        PushOutCircle(playerObj, enemyObj);
                    }
                    else
                    {
                        // (선택) 겹침만 살짝 해소하고 끝내기: 적의 속도는 유지
                        // -> 플레이어를 밀지 않고, 적을 밖으로만 살짝 빼주면 더 안정적
                        // 간단 버전(적을 플레이어 반대 방향으로 2~5 정도 밀기):
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
                        }
                    }
                }
            }

        }

        if (!pushed) break;
    }
    // =========================
    // 敵同士の反動（ノックバック中の敵がぶつかったら受け渡し）
    // =========================
    auto& enemies = m_spawner.GetEnemies();
    for (size_t i = 0; i < enemies.size(); ++i)
    {
        Enemy* a = enemies[i].get();
        if (!a) continue;
        Object* aObj = a->GetObject();
        if (!aObj) continue;

        for (size_t j = i + 1; j < enemies.size(); ++j)
        {
            Enemy* b = enemies[j].get();
            if (!b) continue;
            Object* bObj = b->GetObject();
            if (!bObj) continue;

            if (aObj->CheckCollision(*bObj))
            {
                EnemyReboundBounce(a, aObj, b, bObj);
            }
        }
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

    prevButtons = buttons;
    UpdateUIFollowCamera();

}
void GamePlay::DrawScene()
{
    auto off = m_camera.GetOffset();

    for (auto& obj : objects)
    {
        auto old = obj->GetPos();
        obj->SetPos(old.x + off.x, old.y + off.y, old.z);

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

        obj->SetPos(old.x, old.y, old.z);
    }
}


void GamePlay::UninitScene()
{
    Object::ReleaseTextureCache();
    std::cout << "UninitScene" << std::endl;
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
    auto c = m_camera.GetPosition();

    const float halfW = 835.0f;  // 画面幅 1670 の半分
    const float halfH = 470.0f;  // 画面高さ 940 の半分
    const float pad = 30.0f;   // 画面端からの余白

    // =========================
    // 左上：HPバー（中にアイコンを入れる）
    // =========================

    // HPバーの基準位置（左上に固定）
    const float hpBarX = c.x - halfW + pad + 180.0f; // 左端から少し内側（※見た目に合わせて調整）
    const float hpBarY = c.y + halfH - pad - 44.0f;  // 上端から少し下

    // HPバーを配置
    if (PlayerHeartPointBar)
        PlayerHeartPointBar->SetPos(hpBarX, hpBarY, 0.0f);

    // -------------------------
    // アイコンを「HPバーの左側の丸枠の中」に配置する
    // -------------------------
    // 前提：
    // ・Object の SetPos は「中心座標」
    // ・HPバー画像の左側に丸い枠があるデザイン
    // ・その丸枠の中心位置は「HPバー中心から左へ寄せた位置」
    //
    // ここで使う数値は「HPバーのサイズ」と「丸枠の大きさ」に依存するので、
    // 見た目に合わせて少しずつ調整してください。

    // HPバーの描画サイズ（InitScene で SetSize した値と合わせる）
    const float hpW = 360.0f;    // HPバーの横幅（例：360）
    const float hpH = 48.0f;     // HPバーの高さ（例：48）

    // アイコンのサイズ（丸枠に収まる大きさ）
    const float iconSize = 72.0f;

    // 丸枠の中心を、HPバーの左端から「アイコン半径」分だけ内側に取る
    // ※ circlePadding を増やすと、アイコンが右へ移動（枠の中心に合わせやすい）
    const float circlePadding = 22.0f; 

    const float circleCenterX = hpBarX - (hpW * 0.5f) + (iconSize * 0.5f) + circlePadding;
    const float circleCenterY = hpBarY + 10.0f;

    // アイコンを丸枠の中心へ配置
    if (PlayerIcon)
    {
        PlayerIcon->SetPos(circleCenterX, circleCenterY, 0.0f);
        PlayerIcon->SetSize(iconSize, iconSize, 0.0f);
    }

    // =========================
    // バフ：HPバーの下に横並び
    // =========================
    for (int i = 0; i < (int)BuffIcons.size(); i++)
    {
        if (!BuffIcons[i]) continue;

        // 左上付近で、アイコンの下に横並びで配置
        BuffIcons[i]->SetPos(
            c.x - halfW + pad + 150.0f + (i * 48.0f),   // 横方向：間隔 48
            c.y + halfH - pad - 36.0f - 70.0f,         // 縦方向：HPバーより少し下
            0.0f
        );
    }

    // =========================
    // 右下：ボタン（同じ行）
    // =========================
    if (LightAttackButton)
        LightAttackButton->SetPos(c.x + halfW - pad - 70.0f, c.y - halfH + pad + 30.0f, 0.0f);

    if (HeavyAttackButton)
        HeavyAttackButton->SetPos(c.x + halfW - pad - 160.0f, c.y - halfH + pad + 30.0f, 0.0f);

    // =========================
    // 右上：魔法陣（小さく）
    // =========================
    if (MagicCircle)
        MagicCircle->SetPos(c.x + halfW - pad - 110.0f, c.y + halfH - pad - 110.0f, 0.0f);

    if (ExpBar)
    {
        // HPバーの中心位置(hpBarX, hpBarY)を基準に、少し上へずらす
        // ※ gapY を大きくすると、より上へ移動する
        const float gapY = 10.0f; // 余白（必要に応じて 40～70 で調整）

        ExpBar->SetPos(hpBarX + 85.0f, hpBarY + gapY, 0.0f); // +60 はHPバーに対して右寄せしたい場合
    }
}
static void HeavyPinballHit(Player* playerLogic, Object* playerObj,
    Enemy* enemyLogic, Object* enemyObj)
{
    if (!playerLogic || !playerObj || !enemyLogic || !enemyObj) return;
    if (enemyLogic->IsKnockBacking()) return;

    auto p3 = playerObj->GetPos();
    auto e3 = enemyObj->GetPos();

    DirectX::SimpleMath::Vector2 p(p3.x, p3.y);
    DirectX::SimpleMath::Vector2 e(e3.x, e3.y);

    auto n = e - p;
    if (n.LengthSquared() < 0.0001f)
        n = playerLogic->GetHeavyDashVelocity();

    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    auto vP = playerLogic->GetHeavyDashVelocity();
    float relN = vP.Dot(n);          // ✅ 이 형태가 안전 (C2660 방지)

    if (relN < 50.0f) return;

    // ===== 조절 포인트(속도/거리) =====
    const float baseKick = 100.0f;  // 기본 날리는 속도
    const float kickBySpeed = 0.7f;    // 대쉬가 빠를수록 추가
    const float maxKick = 300.0f;  // 상한

    float kick = baseKick + relN * kickBySpeed;
    if (kick > maxKick) kick = maxKick;

    enemyLogic->KnockBack(n * kick);
}



static void EnemyReboundTransfer(Enemy* a, Object* aObj, Enemy* b, Object* bObj)
{
    if (!a || !b || !aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();

    // 둘 다 날아가거나, 둘 다 그냥 걸으면 처리 안 함
    if (aFly == bFly) return;

    Enemy* fly = aFly ? a : b;      // 날아가던 적
    Enemy* walk = aFly ? b : a;      // 다가오던 적
    Object* flyOb = aFly ? aObj : bObj;
    Object* walkOb = aFly ? bObj : aObj;

    auto v = fly->GetKnockBackVelocity();
    float speed = v.Length();
    if (speed < 1.0f)
    {
        fly->StopKnockBack();
        return;
    }

    // 충돌 방향(날아가던 적 -> 다가오던 적)
    auto fp = flyOb->GetPos();
    auto wp = walkOb->GetPos();
    DirectX::SimpleMath::Vector2 n(wp.x - fp.x, wp.y - fp.y);

    // 완전 겹침이면 속도 방향 사용
    if (n.LengthSquared() < 0.0001f) n = v;
    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    // fly가 실제로 walk 쪽으로 밀고 있을 때만(뒤에서 스치면 무시)
    if (v.Dot(n) <= 0.0f) return;

    // ===== 반동 세기(조절 포인트) =====
    const float reboundRate = 0.85f; // 0~1: 작을수록 반동 약함
    const float minKick = 250.0f;    // 최소 반동 속도
    const float maxKick = 1600.0f;   // 최대 반동 속도

    float kick = speed * reboundRate;
    if (kick < minKick) kick = minKick;
    if (kick > maxKick) kick = maxKick;

    // 1) 날아가던 적 멈춤
    fly->StopKnockBack();

    // 2) 다가오던 적이 반동으로 날아감
    walk->KnockBack(n * kick);

    // 겹침 해소(다음 프레임 연타 충돌 방지): walk만 살짝 밖으로
    float rf = flyOb->GetCollisionRadius();
    float rw = walkOb->GetCollisionRadius();

    float dx = wp.x - fp.x;
    float dy = wp.y - fp.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist < 0.0001f) dist = 0.0001f;

    float overlap = (rf + rw) - dist;
    if (overlap > 0.0f)
    {
        float push = overlap + 2.0f;
        walkOb->SetPos(wp.x + n.x * push, wp.y + n.y * push, wp.z);
    }
}
static void EnemyReboundBounce(Enemy* a, Object* aObj, Enemy* b, Object* bObj)
{
    if (!a || !b || !aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();
    if (!aFly && !bFly) return; // 둘 다 안날아가면 무시

    auto a3 = aObj->GetPos();
    auto b3 = bObj->GetPos();
    DirectX::SimpleMath::Vector2 pa(a3.x, a3.y);
    DirectX::SimpleMath::Vector2 pb(b3.x, b3.y);

    // 충돌 노말( a -> b )
    DirectX::SimpleMath::Vector2 n = pb - pa;
    if (n.LengthSquared() < 0.0001f)
    {
        // 완전 겹치면 상대속도(대충)로 노말을 만든다
        DirectX::SimpleMath::Vector2 rel(0, 0);
        if (aFly) rel += a->GetKnockBackVelocity();
        if (bFly) rel -= b->GetKnockBackVelocity();
        n = (rel.LengthSquared() > 0.0001f) ? rel : DirectX::SimpleMath::Vector2(1, 0);
    }
    n.Normalize();

    // ===== 튜닝 포인트 =====
    const float restitution = 0.85f; // 1.0에 가까울수록 “퐁퐁” 튕김, 0.7~0.9 추천
    const float minSpeed = 150.0f;
    const float maxSpeed = 1600.0f;

    auto bounceOne = [&](Enemy* e, const DirectX::SimpleMath::Vector2& normal)
        {
            auto v = e->GetKnockBackVelocity();

            // normal 방향으로 파고들 때만 튕김 (붙어있을 때 연타 방지)
            if (v.Dot(normal) <= 0.0f) return;

            auto vNew = ReflectVec(v, normal) * restitution;

            float s = vNew.Length();
            if (s < minSpeed) vNew = (-normal) * minSpeed;
            if (s > maxSpeed) vNew *= (maxSpeed / s);

            // ✅ 타이머 유지하고 속도만 변경
            e->SetKnockBackVelocity(vNew);
        };

    if (aFly) bounceOne(a, n);   // a는 +n 기준으로 반사
    if (bFly) bounceOne(b, -n);   // b는 -n 기준으로 반사

    // ===== 겹침 해소(서로 끼어서 떨리는 것 방지) =====
    float ra = aObj->GetCollisionRadius();
    float rb = bObj->GetCollisionRadius();

    DirectX::SimpleMath::Vector2 d = pb - pa;
    float dist = d.Length();
    if (dist < 0.0001f) dist = 0.0001f;

    float overlap = (ra + rb) - dist;
    if (overlap > 0.0f)
    {
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
    if (!a || !b || !aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    const bool aFly = a->IsKnockBacking();
    const bool bFly = b->IsKnockBacking();
    if (!aFly && !bFly) return; // 둘 다 걷는 상태면 무시

    auto ap3 = aObj->GetPos();
    auto bp3 = bObj->GetPos();
    DirectX::SimpleMath::Vector2 ap(ap3.x, ap3.y);
    DirectX::SimpleMath::Vector2 bp(bp3.x, bp3.y);

    // a -> b 방향 노말
    DirectX::SimpleMath::Vector2 n = bp - ap;
    if (n.LengthSquared() < 0.0001f) n = { 1.0f, 0.0f };
    n.Normalize();

    // ==== 튜닝 포인트(여기서 체감이 바뀜) ====
    const float transferRate = 0.95f; // 맞은 적에게 전달 비율 (0~1)
    const float bounceRate = 0.85f; // 튕겨나갈 때 감쇠 (0~1)
    const float minKick = 700.0f; // 맞은 적 최소 속도
    const float maxKick = 1100.0f; // 맞은 적 최대 속도
    const float stopSpeed = 10.0f;  // 이 이하로 느리면 멈춘 것으로 처리(선택)

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
                float push = overlap + 2.0f; // +2는 재충돌 방지 여유
                // 둘 다 반씩 밀어서 분리 (연타 충돌 방지에 중요)
                o1->SetPos(p1.x - normal.x * (push * 0.5f), p1.y - normal.y * (push * 0.5f), p1.z);
                o2->SetPos(p2.x + normal.x * (push * 0.5f), p2.y + normal.y * (push * 0.5f), p2.z);
            }
        };

    auto BounceOne = [&](Enemy* fly, Object* flyObj, Enemy* other, Object* otherObj, DirectX::SimpleMath::Vector2 normalFlyToOther)
        {
            auto v = fly->GetKnockBackVelocity();
            float vn = v.Dot(normalFlyToOther);

            // ✅ 이게 네가 물어본 "if (v.Dot(normal) <= 0.0f) return;" 같은 역할
            // 상대쪽으로 실제로 박고 있을 때만 처리 (떨어지는 중이면 무시)
            if (vn <= 0.0f) return;

            // 1) 맞은 적에게 전달
            float kick = ClampF(vn * transferRate, minKick, maxKick);
            other->KnockBack(normalFlyToOther * kick);

            // 2) 날아가던 적은 "반사" (거울 반사: v - 2*(v·n)*n)
            DirectX::SimpleMath::Vector2 vRef = v - 2.0f * vn * normalFlyToOther;
            vRef *= bounceRate;

            if (vRef.Length() < stopSpeed)
            {
                // 너무 느리면 멈추게(원하면 삭제해도 됨)
                fly->StopKnockBack();
            }
            else
            {
                // ✅ 중요: 여기서 Stop하지 말고 속도만 바꿔서 계속 날아가게
                fly->SetKnockBackVelocity(vRef);
            }

            // 3) 겹침 해소 (이거 없으면 매 프레임 계속 튕김/전달 반복해서 폭주함)
            ResolveOverlap(flyObj, otherObj, normalFlyToOther);
        };

    if (aFly && !bFly)
    {
        BounceOne(a, aObj, b, bObj, n);
    }
    else if (!aFly && bFly)
    {
        BounceOne(b, bObj, a, aObj, -n);
    }
    else
    {
        // 둘 다 날아가는 상태면: 서로 튕기게 (간단 버전: 서로 반사)
        // (원하면 더 물리적인 탄성충돌로 바꿀 수도 있음)
        auto va = a->GetKnockBackVelocity();
        auto vb = b->GetKnockBackVelocity();

        float vna = va.Dot(n);
        float vnb = vb.Dot(n);

        // 서로 접근 중일 때만
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
