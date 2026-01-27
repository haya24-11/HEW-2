#pragma once
#include <memory>
#include "Scene.h"
#include "Player.h"
#include "Enemy.h"
#include "Camera2D.h"
class GamePlay :
    public Scene
{
public:
    GamePlay();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

private:
    // メンバ変数
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Enemy>  m_enemy;

    Camera2D m_camera;
};

