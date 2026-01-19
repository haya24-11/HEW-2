#pragma once
#include <vector>
#include <memory>
#include "SceneTypes.h"
#include "GameObject.h"

class Scene
{
private:
    bool isChange = false;
    SceneType nextScene = SceneType::Title;

protected:
    void ChangeScene(SceneType next);
    void SetNextScene(SceneType nextScene);

    std::vector<std::unique_ptr<GameObject>> objects;//オブジェクト格納
    int objectNum = 0;//初期オブジェクト数

public:
    Scene(SceneType type);
    virtual ~Scene() = default;

    virtual void InitScene() = 0;
    virtual void UpdateScene(float deltaTime) = 0;
    virtual void DrawScene() = 0;
    virtual void UninitScene() = 0;

    bool IsChange() const;
    SceneType GetNextScene() const;
};
