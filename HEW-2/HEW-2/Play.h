#pragma once
#include "Scene.h"
class Play :
    public Scene
{
public:
    Play();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

