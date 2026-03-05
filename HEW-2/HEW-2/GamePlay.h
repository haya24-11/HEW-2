#pragma once
#include <memory>
#include <vector>

#include "Scene.h"
#include "Player.h"
#include "Camera2D.h"
#include "EnemySpawner.h"
#include "ComboManager.h"
#include "SkillSelectUI.h"

class AttackSlashEffect; // 前方宣言

class GamePlay : public Scene
{
public:
    GamePlay();

    // ゲームプレイUIオブジェクト一覧（必ず nullptr 初期化）
    Object* LightAttackButton = nullptr; // 弱攻撃ボタン
    Object* HeavyAttackButton = nullptr; // 強攻撃ボタン
    std::vector<Object*> BuffIcons;           // バフアイコン一覧
    Object* BuffIcon = nullptr; // バフアイコン
    Object* BuffIcon_A = nullptr; // バフアイコン
    Object* PlayerIcon = nullptr; // プレイヤーの状態アイコン
    Object* MagicCircle = nullptr; // 魔法陣表示
    Object* PlayerHeartPointBar = nullptr; // プレイヤーHP
    Object* ExpBarBack = nullptr; // 経験値ゲージ（背景）
    Object* ExpBarGauge = nullptr; // 経験値ゲージ（ゲージ）
    Object* ExpBarFrame = nullptr; // 経験値ゲージ（フレーム）
    //ui
    Object* UI_KeyboardGuide = nullptr;
    Object* UI_PadGuide = nullptr;

    enum class InputDevice { Keyboard, Pad };
    InputDevice m_lastInput = InputDevice::Keyboard;

    WORD m_prevPadButtonsUI = 0;
    // シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

    void UpdateUIFollowCamera();

    //一知事停止、再開
    void Pause() { m_paused = true; }
    void Resume() { m_paused = false; }

    // combo
    ComboManager& GetCombo() { return m_combo; }
    Player* GetPlayer() const { return m_player.get(); }

    // 通常攻撃
    EnemySpawner& GetSpawner() { return m_spawner; }
    const EnemySpawner& GetSpawner() const { return m_spawner; }

private:
    std::unique_ptr<Player> m_player;
    EnemySpawner m_spawner;
    Camera2D m_camera;

    ComboManager m_combo;

    bool m_paused = false;
    SkillSelectUI* m_skillUI = nullptr;

    Object* m_map = nullptr;

    int m_playtime = 0; //ゲームプレイ時間(Result掲載用)
    float m_rotation = 0.0f;
    float m_rotationSpeed = 7.5f;

    // ボスタイマー
    float m_bossTimer = 0.0f;
    bool  m_bossPhase = false;
    bool  m_bossHasSpawned = false;

    std::vector<std::unique_ptr<AttackSlashEffect>> m_attackEffects;
    std::vector<Object*> m_levelDigits;
    Object* m_levelLabel = nullptr;   // "LEVEL." 表示用

    // 生存タイマー
    float m_timer = 0.0f;
    // タイマーUI
    std::vector<Object*> m_timerDigits;
    Object* m_timerColon = nullptr;
    // リザルトにタイムを渡す
    ResultData m_resultData;
};