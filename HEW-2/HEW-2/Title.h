#pragma once
#include "Scene.h"
class Title :
    public Scene
{
public:
    Title();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

