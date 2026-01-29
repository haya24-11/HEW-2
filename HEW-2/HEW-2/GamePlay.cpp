#include "GamePlay.h"
#include "input.h"
#include "NormalEnemy.h"
#include "Camera2D.h"
#include <cmath>
#include "Texture.h"
#include "CameraGlobals.h" 
//XINPUT
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

static void PushOutCircle(Object* playerObj, Object* enemyObj);


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
    player->SetCollisionRadius(50.0f);

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
                PushOutCircle(playerObj, enemyObj);
                pushed = true;
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
