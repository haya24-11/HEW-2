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

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

private:
    std::unique_ptr<Player> m_player;
    EnemySpawner m_spawner;
    Camera2D m_camera;
};
