#pragma once
#include "Scene.h"
class GameOver :
    public Scene
{
public:
    GameOver();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

