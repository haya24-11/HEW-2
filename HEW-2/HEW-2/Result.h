#pragma once
#include "Scene.h"
class Result :
    public Scene
{
public:
    Result();

    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

};

