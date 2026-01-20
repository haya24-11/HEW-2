#pragma once
#include "Scene.h"
class GameOver :
    public Scene
{

    void InitScene() override = 0;
    void UpdateScene(float deltaTime) override = 0;
    void DrawScene() override = 0;
    void UninitScene() override = 0;

};

