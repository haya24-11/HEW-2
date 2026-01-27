#pragma once
#include <memory>
#include "Scene.h"
#include "Player.h"
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
    // ƒƒ“ƒo•Ï”
    std::unique_ptr<Player> m_player;
};

