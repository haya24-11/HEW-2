#pragma once
#include <memory>
#include "Scene.h"
#include "Player.h"
#include "Camera2D.h"
#include "EnemySpawner.h"

class GamePlay : public Scene
{
public:
    GamePlay();
        

    //ゲームプレイUIオブジェクト一覧
    Object* LightAttackButton; //弱攻撃ボタン
    Object* HeavyAttackButton; //強攻撃ボタン
    std::vector<Object*> BuffIcons;
    Object* BuffIcon; //バフアイコン
    Object* BuffIcon_A; //バフアイコン
    Object* PlayerIcon; //プレイヤーの状態アイコン
    Object* MagicCircle; //選択したモードの魔法陣を表示
    Object* PlayerHeartPointBar; //プレイヤーのHP
    // Object* EnemyHeartPointBar; //敵のHP
    Object* ExpBar; //経験値ゲージ
    //Object* Combo; //コンボ表示
    //Object* Player; //プレイヤー
    //Object* Enemy; //敵


    //シーンの動作
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

    void UpdateUIFollowCamera();

private:
    std::unique_ptr<Player> m_player;
    EnemySpawner m_spawner;
    Camera2D m_camera;
};
