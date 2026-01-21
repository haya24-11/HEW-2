#pragma once
#include "Scene.h"
class GamePlay :
    public Scene
{
public:
    GamePlay();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

