#pragma once
#include <memory>
#include <vector>

#include "Scene.h"
#include "Player.h"
#include "Camera2D.h"
#include "EnemySpawner.h"
#include "ComboManager.h"

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

    // シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

    void UpdateUIFollowCamera();

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

    Object* m_map = nullptr;

    float m_rotation = 0.0f;
    float m_rotationSpeed = 7.5f;

    // ボスタイマー
    float m_bossTimer = 0.0f;
    bool  m_bossPhase = false;
    bool  m_bossHasSpawned = false;

    std::vector<AttackSlashEffect*> m_attackEffects;
    std::vector<Object*> m_levelDigits;
};