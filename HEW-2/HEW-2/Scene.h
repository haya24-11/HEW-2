#pragma once
#include <vector>
#include <memory>
#include "GameTypes.h"

class GameObject;

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void OnEnter() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnDraw() = 0;
    virtual void OnExit() = 0;

    bool IsEnd() const;
    SceneType GetNextScene() const;

protected:
    void EndScene(SceneType next);

    std::vector<std::unique_ptr<GameObject>> objects;

private:
    bool isEnd = false;
    SceneType nextScene = SceneType::Title;
};
