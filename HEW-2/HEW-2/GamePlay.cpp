#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

#ifdef GetObject
#undef GetObject
#endif
#include <iostream>
#include "GamePlay.h"
#include "Player.h"
#include "Enemy.h"
#include "NormalEnemy.h"
#include "NormalEnemyred.h"
#include "NormalEnemyblue.h"
#include "Texture.h"
#include "CameraGlobals.h"
#include <cmath>
#include <algorithm> // std::clamp
#include <unordered_map>
#include <cmath> // sqrtf
#include <unordered_map>

static std::unordered_map<Enemy*, float> s_touchHitCD;
// 「接触」判定（CheckCollisionが厳しい時の保険）
static bool IsTouching(Object* a, Object* b, float margin = 10.0f)
{
    if (!a || !b) return false;

    auto pa = a->GetPos();
    auto pb = b->GetPos();

    float dx = pb.x - pa.x;
    float dy = pb.y - pa.y;

    float r = a->GetCollisionRadius() + b->GetCollisionRadius() + margin;
    return (dx * dx + dy * dy) <= (r * r);
}

// 接触デバッグ表示（毎フレーム出ると重いので「たまに」出す）


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

static float s_touchHitCD_All = 0.0f; // 全モンスター共通の接触被弾CT

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
    // ===== リプレイ対策：完全リセット（InitSceneの先頭）=====
    LightAttackButton = nullptr;
    HeavyAttackButton = nullptr;
    BuffIcons.clear();
    BuffIcon = nullptr;
    BuffIcon_A = nullptr;
    PlayerIcon = nullptr;
    MagicCircle = nullptr;
    PlayerHeartPointBar = nullptr;
    ExpBarBack = nullptr;
    ExpBarGauge = nullptr;
    ExpBarFrame = nullptr;
    m_levelDigits.clear();

    // エフェクト完全削除
    for (auto& e : m_attackEffects)
    {
        if (e)
        {
            e->Uninit();
        }
    }
    m_attackEffects.clear();

    // ボス関連
    m_bossTimer = 0.0f;
    m_bossPhase = false;
    m_bossHasSpawned = false;

    // プレイヤー/マップ
    m_player.reset();
    m_map = nullptr;

    // ✅ 重要：Spawnerを作り直す（中のポインタ/敵リストを完全リセット）
    m_spawner = EnemySpawner();

    // Comboも念のため作り直し（または Reset() を用意）
   // m_combo = ComboManager();

    // カメラも初期値に（必要なら）
    m_camera = Camera2D();

    PreloadTexture(g_pDevice, "asset/Texture/BossWalk.png");        
    //PreloadTexture(g_pDevice, "asset/Texture/boss_attack.png");
   // PreloadTexture(g_pDevice, "asset/Texture/boss_die.png");

    PreloadTexture(g_pDevice, "asset/Texture/ComboEffect.png");
    // ===== プレイヤー関連テクスチャを事前ロード =====
    PreloadTexture(g_pDevice, "asset/Texture/player_idle.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_walk.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_light.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_heavy.png");
    PreloadTexture(g_pDevice, "asset/Texture/slash_effect.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_damaged.png");
    PreloadTexture(g_pDevice, "asset/UI/LevelNumber.png");
    PreloadTexture(g_pDevice, "asset/UI/level_text.png");

    Object dummy;
    dummy.Init("asset/Texture/player_attack_heavy.png", 1, 1);
    //dummy.Init("asset / Texture / BossWalk.png" , 1, 1);
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
    m_player->SetGamePlay(this);

    // ===== Player Object =====
    Object* player = AddObject();
    player->Init("asset/Texture/player_idle.png");
    player->SetSpriteSheet(6, 6);
    player->SetSize(150.0f, 170.0f, 0.0f);  // ★2回入ってたので1つに整理
    player->SetPos(0.0f, 0.0f, 0.0f);
    player->SetCollisionRadius(70.0f);

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
    m_spawner.Init(
        this,
        m_player->GetObject(),
        m_player.get()      // Player本体を渡す
    );
    m_spawner.RegisterType<NormalEnemy>(1.0f);
    m_spawner.RegisterType<NormalEnemyred>(1.0f);
    m_spawner.RegisterType<NormalEnemyblue>(1.0f);
    std::cout << "(Debug) GamePlayScene!" << std::endl;

    /////////////////////////////////////////////////////////////////////////////
    // UIのPOS設定はここではなく UpdateUIFollowCamera() で毎フレーム行う
    /////////////////////////////////////////////////////////////////////////////

    // ===== 弱攻撃ボタンUI =====
    /*LightAttackButton = AddObject()
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
    */
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
        ->SetSize(0.0f, 40.0f, 0.0f)   // ※見やすい太さ（必要なら調整）
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

    m_combo.Init(this);

    for (int i = 0; i < 2; i++)
    {
        Object* digit = AddObject();
        digit->Init("asset/UI/LevelNumber.png", 5, 2);
        digit->SetSpriteSheet(5, 2);
        digit->SetUI(true);
        digit->SetSize(96.0f, 96.0f, 0.0f);  // ←縦長なので少し縦強め

        m_levelDigits.push_back(digit);
    }

    // ============================
    // LEVEL. ラベル
    // ============================
    m_levelLabel = AddObject();
    m_levelLabel->Init("asset/UI/level_text.png");  // ← LEVEL. 画像パス
    m_levelLabel->SetUI(true);
    m_levelLabel->SetSize(120.0f, 30.0f, 0.0f);  // ★ここ調整ポイント①


    //2026 3 5 ui
 
    // =========================
// 左下キーガイドUI
// =========================

// 1段目（A D W S）
    UI_KeyboardGuide = AddObject()
        ->SetPos(-490.0f, -410.0f, 0.0f)
        ->SetSize(700.0f, 100.0f, 0.0f)
        ->SetAngle(0.0f);
    UI_KeyboardGuide->Init("asset/UI/Keyboard.png");
    UI_KeyboardGuide->SetUI(true);

    UI_PadGuide = AddObject()
        ->SetPos(-590.0f, -410.0f, 0.0f)
        ->SetSize(500.0f, 100.0f, 0.0f)
        ->SetAngle(0.0f);
    UI_PadGuide->Init("asset/UI/Pad.png");
    UI_PadGuide->SetUI(true);

    UI_KeyboardGuide->SetColor(1, 1, 1, 1.0f);
    UI_PadGuide->SetColor(1, 1, 1, 0.0f);

    m_lastInput = InputDevice::Keyboard;
    m_prevPadButtonsUI = 0;
    // ============================
    // タイマーUI
    // ============================

    // 数字 (MMSS → 4桁)
    for (int i = 0; i < 4; i++)
    {
        Object* digit = AddObject();
        digit->Init("asset/UI/timer_number.png", 5, 2); // 0〜9横並び
        digit->SetSpriteSheet(5, 2);
        digit->SetUI(true);
        digit->SetSize(96.0f, 96.0f, 0.0f);

        m_timerDigits.push_back(digit);
    }

    // 「:」
    m_timerColon = AddObject();
    m_timerColon->Init("asset/UI/colon.png");
    m_timerColon->SetUI(true);
    m_timerColon->SetSize(96.0f, 96.0f, 0.0f);

    m_timer = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        if (i < m_timerDigits.size())
            m_timerDigits[i]->SetAnimFrame(0);
    }

    // ★重要：最初のフレームからUI位置を確定（2回目開始のズレ防止）
    UpdateUIFollowCamera();
}


void GamePlay::UpdateScene(float deltaTime)
{

    // ==========================================
    // ✅ 入力デバイス判定用（Keyboard / Pad）
    // ==========================================
    auto AnyKeyboardInput = [&]() -> bool
        {
            // WASD
            if (Input::GetKeyTrigger('W')) return true;
            if (Input::GetKeyTrigger('A')) return true;
            if (Input::GetKeyTrigger('S')) return true;
            if (Input::GetKeyTrigger('D')) return true;

            // よく使うキー
            if (Input::GetKeyTrigger(VK_RETURN)) return true;
            if (Input::GetKeyTrigger(VK_ESCAPE)) return true;
            if (Input::GetKeyTrigger(VK_SHIFT))  return true;
            if (Input::GetKeyTrigger(VK_SPACE))  return true;

            return false;
        };

    auto AnyPadInput = [&]() -> bool
        {
            XINPUT_STATE st{};
            if (XInputGetState(0, &st) != ERROR_SUCCESS) return false;

            // 🔴 修正：ここで m_skillUI->Update は絶対に呼ばない！
            if (m_paused) return false;

            const WORD buttons = st.Gamepad.wButtons;
            // ... 以下ボタン判定 ...
        };

    // 2. UpdateScene の冒頭にある UI 更新処理を「さらに」厳重にする
    if (m_skillUI != nullptr)
    {
        SkillSelectUI* pActiveUI = m_skillUI;
        pActiveUI->Update(deltaTime);

        // 🔴 修正：UI 内で ClearSkillUI が呼ばれたら、即座に return して
        // 後の AnyPadInput などに一切触れさせない
        if (m_skillUI == nullptr) {
            return;
        }
        return;
    }

    // ★ ここで UI を安全に更新
    if (m_skillUI != nullptr)
    {
        // 別の変数にコピーして実行（実行中に m_skillUI が nullptr になっても大丈夫なようにする）
        SkillSelectUI* pActiveUI = m_skillUI;
        pActiveUI->Update(deltaTime);

        // UI 内で決定され、m_skillUI が nullptr になっていたら、このフレームはここで終了
        if (m_skillUI == nullptr) {
            return;
        }

        return; // UI 表示中は、これ以降のゲームロジック（エネミー等）を一切動かさない
    }


    // ==========================================
    // deltaTime 調整
    // ==========================================
    float realDT = deltaTime;
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    
    // =======================================
    // デバッグ操作
    // =======================================

    // F1 : タイム +10秒
    if (Input::GetKeyTrigger(VK_F1))
    {
        m_timer += 10.0f;

        std::cout << "[DEBUG] Time = " << m_timer << std::endl;
    }

    // F2 : 討伐数 +10
    if (Input::GetKeyTrigger(VK_F2))
    {
        m_spawner.DebugAddKill(10);
    }

    // F3 : コンボ +1
    if (Input::GetKeyTrigger(VK_F3))
    {
        m_combo.AddCombo(1);
    }


    // =============================
    // 生存タイマー
    // =============================
    {
        // 更に遅くさせてリアルタイムの時間速度に合わせるための変数
        float timerSpeed = 0.15f;   // ★ここで遅さ調整（0.5 = 半分の速度）

        float add = deltaTime * timerSpeed;

        if (add > 0.1f)
            add = 0.1f;

        m_timer += add;

        int totalSec = (int)m_timer;

        // 安全制限（極端な低FPS対策）
        if (totalSec > 3599)
            totalSec = 3599;

        int minute = totalSec / 60;
        int second = totalSec % 60;

        int digits[4] =
        {
            minute / 10,
            minute % 10,
            second / 10,
            second % 10
        };

        // UI反映
        for (int i = 0; i < 4; i++)
        {
            if (i >= m_timerDigits.size()) break;
            m_timerDigits[i]->SetAnimFrame(digits[i]);
        }
    }
    const bool wasHeavyDashing = m_player->IsHeavyDashing();

    m_player->Update(deltaTime);

    // ★ レベルアップ検知
    if (m_player->IsJustLeveledUp())
    {
        Pause();

        // 1. 候補を取得（最大2つ）
        std::vector<Skill*> options = m_player->GetRandomSkillChoices(2);

        if (options.empty())
        {
            // 🔴 全てLv3なら何もしない
            m_player->ResetLevelUpFlag();
        }
        else if (options.size() == 1)
        {
            // 🔴 残り1つなら強制取得（UIを出さない）
              // 🔴 修正：options そのものではなく、0番目の要素を渡す
            m_player->ApplyAbility(options[0]);

            // 念のため、適用後にゲームが止まらないようにフラグをリセット
            m_player->ResetLevelUpFlag();

            // ログ出力（これが出れば成功）
            printf("[DEBUG] Auto-applied last skill: %p\n", options[0]);
        }
        else
        {
            // 🔴 2つ以上あるなら通常通り選択UIを出す
            Pause();
            m_skillUI = new SkillSelectUI(this, options);
            m_player->ResetLevelUpFlag();
        }

    }

    //combo
    m_combo.Update(deltaTime);

    // ===== 強攻撃ダッシュが「今」終わった瞬間を検出
    const bool isHeavyDashing = m_player->IsHeavyDashing();

    // ✅ ここに追加：強攻撃ダッシュが「今」始まった瞬間 → コンボ開始
    if (!wasHeavyDashing && isHeavyDashing)
    {
        m_combo.BeginAttack();
    }

    if (wasHeavyDashing && !isHeavyDashing)
    {
        m_player->StartNoHitAnim(1.0f);
    }

    if (!m_player) return;

    Object* playerObj = m_player->GetObject();
    if (!playerObj) return;

    const auto oldPos = playerObj->GetPos();

    // ==========================================
    // ✅ 入力デバイス判定（ここでUIを切り替える）
    //  - パッド入力があれば Pad 表示
    //  - それ以外でキーボード入力があれば Keyboard 表示
    // ==========================================
    {
        const bool padNow = AnyPadInput();
        const bool keyNow = AnyKeyboardInput();

        if (padNow)
            m_lastInput = InputDevice::Pad;
        else if (keyNow)
            m_lastInput = InputDevice::Keyboard;

        // 表示切り替え（アルファ）
        if (UI_KeyboardGuide && UI_PadGuide)
        {
            if (m_lastInput == InputDevice::Keyboard)
            {
                UI_KeyboardGuide->SetColor(1, 1, 1, 1.0f);
                UI_PadGuide->SetColor(1, 1, 1, 0.0f);
            }
            else
            {
                UI_KeyboardGuide->SetColor(1, 1, 1, 0.0f);
                UI_PadGuide->SetColor(1, 1, 1, 1.0f);
            }
        }

        // 次フレーム用に pad buttons を保存
        XINPUT_STATE st{};
        if (XInputGetState(0, &st) == ERROR_SUCCESS)
            m_prevPadButtonsUI = st.Gamepad.wButtons;
        else
            m_prevPadButtonsUI = 0;
    }

    // ==========================================
    // 弱攻撃エフェクト
    // ==========================================
    bool attackStart = m_player->ConsumeAttackEffectRequest();

    if (attackStart)
    {
        m_combo.BeginAttack(); // ★コンボ開始
    }

    if (attackStart)
    {
        // ✅ 強攻撃中（チャージ/ダッシュ）は AttackSlashEffect を出さない
        const bool isHeavy = m_player->IsHeavyCharging() || m_player->IsHeavyDashing();

        if (!isHeavy)
        {
            m_attackEffects.push_back(
                std::make_unique<AttackSlashEffect>(
                    this,
                    m_player->GetObject(),
                    m_player->GetAttackDir(),
                    m_player->IsFacingRight(),
                    m_player->GetPower()
                )
            );
        }
    }

    // 全体接触CD
    if (s_touchHitCD_All > 0.0f)
    {
        s_touchHitCD_All -= deltaTime;
        if (s_touchHitCD_All < 0.0f) s_touchHitCD_All = 0.0f;
    }

    // ==========================================
    // 攻撃エフェクトの更新＆破棄
    // ==========================================
    for (auto it = m_attackEffects.begin(); it != m_attackEffects.end(); )
    {
        // 🔴 1. まず中身が空でないか、不正なアドレスでないか超厳重チェック
        if (it->get() == nullptr || it->get() == (void*)0xFFFFFFFFFFFFFFFF)
        {
            it = m_attackEffects.erase(it);
            continue;
        }

        // 🔴 2. 更新処理を呼ぶ。この中で Uninit() されても Dead フラグで判定する
        (*it)->Update(deltaTime);

        // 🔴 3. 死んだエフェクトを安全に削除
        if ((*it)->IsDead())
        {
            it = m_attackEffects.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // ==========================================
    // スポーナー更新
    // ==========================================
    m_spawner.Update(deltaTime);

    // 強攻撃ヒットCD更新
    for (auto it = s_heavyHitCD.begin(); it != s_heavyHitCD.end(); )
    {
        it->second -= deltaTime;
        if (it->second <= 0.0f) it = s_heavyHitCD.erase(it);
        else ++it;
    }

    // 接触ヒットCD更新（敵ごと）
    for (auto it = s_touchHitCD.begin(); it != s_touchHitCD.end(); )
    {
        it->second -= deltaTime;
        if (it->second <= 0.0f) it = s_touchHitCD.erase(it);
        else ++it;
    }

    // ==========================================
    // 魔法陣回転
    // ==========================================
    m_rotation += m_rotationSpeed * deltaTime;
    MagicCircle->SetAngle(m_rotation);

    if (m_rotation >= 360.0f)
        m_rotation -= 360.0f;

    // ==========================================
    // プレイヤー vs 敵：接触判定・押し出し（3回反復）
    // ==========================================
    for (int iter = 0; iter < 3; ++iter)
    {
        bool pushed = false;

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;

            Object* enemyObj = e->GetObject();
            if (!enemyObj) continue;

            const float kTouchMargin = 10.0f;

            const bool chk = playerObj->CheckCollision(*enemyObj);
            const bool touch = IsTouching(playerObj, enemyObj, kTouchMargin);
            const bool contact = chk || touch;

            if (!contact) continue;

            if (m_player->IsHeavyDashing())
            {
                HeavyPinballHit(m_player.get(), playerObj, e.get(), enemyObj);
                pushed = true;
            }
            else if (m_player->IsHeavyCharging())
            {
                PushOutCircle(playerObj, enemyObj);
                pushed = true;
            }
            else
            {
                if (!m_player->IsNoHitAnim())
                {
                    const float touchCooldown = 10.0f;

                    if (s_touchHitCD_All <= 0.0f)
                    {
                        m_player->TakeDamage(e->GetPower());
                        s_touchHitCD_All = touchCooldown;
                    }
                }

                if (!e->IsKnockBacking())
                {
                    PushOutCircle(playerObj, enemyObj);
                    pushed = true;
                }
                else
                {
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

        if (!pushed) break;
    }

    // ==========================================
    // PAD処理シ－ン切り替え
    // ==========================================
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
        m_resultData.monsterKills = m_spawner.GetKillCount();
        m_resultData.maxCombo = m_combo.GetMaxCombo();
        m_resultData.playTime = m_timer;
        m_resultData.isClear = true;

        Scene::SetResultData(m_resultData);

        SetNextScene(SceneType::Result);
    }

    // ==========================================
    // ✅ すべての移動/衝突処理が終わった「最後」にカメラを確定する
    // ==========================================
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

            if (loX > hiX) camX = (left + right) * 0.5f;
            else           camX = ClampFloat(camX, loX, hiX);

            if (loY > hiY) camY = (bottom + top) * 0.5f;
            else           camY = ClampFloat(camY, loY, hiY);
        }

        m_camera.SetPosition({ camX, camY });
        g_cameraX = camX;
        g_cameraY = camY;
    }

    // ==========================================
    // ボス死亡→Result
    // ==========================================
    {
        bool bossFound = false;
        bool bossAlive = false;

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;

            if (e->IsBoss())
            {
                bossFound = true;
                m_bossHasSpawned = true;

                if (e->IsAlive())
                    bossAlive = true;

                break;
            }
        }

        if (m_bossHasSpawned && (!bossFound || !bossAlive))
        {
            m_resultData.monsterKills = m_spawner.GetKillCount();
            m_resultData.maxCombo = m_combo.GetMaxCombo();
            m_resultData.playTime = m_timer;
            m_resultData.isClear = true;

            Scene::SetResultData(m_resultData);
            SetNextScene(SceneType::Result);
            return;
        }
    }

    if (m_player->GetHp() <= 0)
    {

        SetNextScene(SceneType::GameOver);
        return;
    }

    // ==========================================
    // マップ外Clamp
    // ==========================================
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

    // =======================================
    // EXPバー更新
    // =======================================
    if (m_player && ExpBarGauge)
    {
        float current = (float)m_player->GetCurrentExp();
        float next = (float)m_player->GetNextLevelExp();

        float rate = 0.0f;
        if (next > 0.0f)
            rate = current / next;

        rate = std::clamp(rate, 0.0f, 1.0f);

        const float maxWidth = 1600.0f;
        float width = maxWidth * rate;

        ExpBarGauge->SetSize(width, 40.0f, 0.0f);
    }

    // =======================================
    // レベル数字 左上固定表示
    // =======================================
    int level = m_player->GetLevel();

    for (auto d : m_levelDigits)
        d->SetActive(false);

    std::string levelStr = std::to_string(level);

    float halfW = SCREEN_WIDTH * 0.5f;
    float halfH = SCREEN_HEIGHT * 0.5f;

    float startX = -halfW + 380.0f;
    float startY = halfH - 70.0f;

    float digitWidth = 96.0f;
    float spacing = digitWidth - 67.0f;

    for (int i = 0; i < (int)levelStr.size(); i++)
    {
        if (i >= (int)m_levelDigits.size()) break;

        int num = levelStr[i] - '0';

        auto obj = m_levelDigits[i];
        obj->SetActive(true);
        obj->SetAnimFrame(num);

        obj->SetPos(startX + i * spacing, startY, 0.0f);
    }

    if (m_levelLabel)
    {
        m_levelLabel->SetPos(startX - 90.0f, startY, 0.0f);
    }

    for (int i = 0; i < (int)m_levelDigits.size(); i++)
    {
        if (i < (int)levelStr.size())
        {
            m_levelDigits[i]->SetActive(true);
            m_levelDigits[i]->SetAnimFrame(levelStr[i] - '0');
        }
        else
        {
            m_levelDigits[i]->SetActive(false);
        }
    }

    prevButtons = buttons;

    UpdateUIFollowCamera();
}

void GamePlay::DrawScene()
{
    // =============================
    // ① ワールド描画（UI以外）
    // =============================
    for (auto& obj : objects)
    {
        if (obj->IsUI()) continue;

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

        ExpBarFrame->Draw();
        ExpBarGauge->Draw();
        ExpBarBack->Draw();

    }

    // =============================
    // UI描画（常に最前面）UI位置更新は描画前に1回だけ
    // =============================
    for (auto& obj : objects)
    {
        if (!obj->IsUI()) continue;
        obj->Draw();
    }

}

void GamePlay::UninitScene()
{
    for (auto& e : m_attackEffects) {
        if (e) e->Uninit();
    }
    m_attackEffects.clear(); // リストを空にする

   // m_combo = ComboManager();

    if (m_skillUI)
    {
        m_skillUI->Uninit();
        delete m_skillUI;
        m_skillUI = nullptr;
    }


    std::cout << "UninitScene(GamePlay)" << std::endl;

    // ✅ 攻撃エフェクトを確実に破棄（Scene内Objectも安全に除去）
    for (auto& e : m_attackEffects)
    {
        if (e)
        {
            e->Uninit();
        }
    }

    // ✅ Enemy* をキーにした静的クールダウンはリプレイで必ずクリア（アドレス再利用でバグる）
    s_touchHitCD.clear();
    s_heavyHitCD.clear();
    s_touchHitCD_All = 0.0f;

    // ✅ 参照を切る（ClearObjectでObjectは消えるので、ポインタを残すと危険）
    LightAttackButton = nullptr;
    HeavyAttackButton = nullptr;
    BuffIcons.clear();
    BuffIcon = nullptr;
    BuffIcon_A = nullptr;
    PlayerIcon = nullptr;
    MagicCircle = nullptr;
    PlayerHeartPointBar = nullptr;
    ExpBarBack = nullptr;
    ExpBarGauge = nullptr;
    ExpBarFrame = nullptr;
    m_levelDigits.clear();

    m_player.reset();
    m_map = nullptr;

    m_timerDigits.clear();
    m_timerColon = nullptr;

    // ✅ Spawnerも念のため初期化（内部Enemy配列を破棄）
    m_spawner = EnemySpawner();
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
        PlayerHeartPointBar->SetPos(hpBarX, hpBarY - 20, 0.0f);

    // アイコン
    const float hpW = 360.0f;
    const float iconSize = 72.0f;
    const float circlePadding = 22.0f;

    const float circleCenterX = hpBarX - (hpW * 0.5f) + (iconSize * 0.5f) + circlePadding;
    const float circleCenterY = hpBarY + 10.0f;


    if (PlayerIcon)
    {
        PlayerIcon->SetPos(circleCenterX + 0, circleCenterY - 10, 0.0f);
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
        ExpBarBack->SetPos(hpBarX + 665.0f, hpBarY + gapY, 0.0f);     // 経験値バー 背景
        // ==========================
        // EXPバー左端固定
        // ==========================
        /*
        hpBarX            // カメラ基準位置
        + 665.0f          // ← UI横オフセット（これが位置調整）
        - (1600 * 0.5f)   // 画面中央補正
        */
        const float expBarLeft =
            hpBarX + 610.0f - (1600.0f * 0.5f);

        float gaugeWidth = ExpBarGauge->GetSize().x;

        // 中心座標 = 左端 + 半分
        ExpBarGauge->SetPos(
            expBarLeft + gaugeWidth * 0.5f,
            hpBarY + gapY,
            0.0f
        );
        ExpBarFrame->SetPos(hpBarX + 665.0f, hpBarY + gapY, 0.0f);  // 経験値バー フレーム
    }

    // ============================
    // タイマーUI（画面上中央）
    // ============================
    {
        float timerY = halfH - 60.0f;   // ← 上からの位置調整
        float timerX = 0.0f;            // ← 画面中央

        float digitWidth = 96.0f;       // ← 数字サイズ
        float spacing = digitWidth - 30.0f;   // ← ★数字間隔調整ポイント

        for (int i = 0; i < m_timerDigits.size(); i++)
        {
            if (!m_timerDigits[i]) continue;

            float x = timerX + (i - 1.5f) * spacing;

            m_timerDigits[i]->SetPos(
                x,
                timerY,
                0.0f
            );
        }

        // コロン
        if (m_timerColon)
        {
            m_timerColon->SetPos(
                timerX,
                timerY,
                0.0f
            );
        }
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
    playerLogic->GetGamePlay()->GetCombo().AddHit();

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
            fly->GetGamePlay()->GetCombo().AddHit();

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
