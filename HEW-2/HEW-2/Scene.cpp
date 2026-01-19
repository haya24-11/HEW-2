#include "Scene.h"

bool Scene::IsEnd() const
{
    return isEnd;
}

SceneType Scene::GetNextScene() const
{
    return nextScene;
}

void Scene::EndScene(SceneType next)
{
    isEnd = true;
    nextScene = next;
}