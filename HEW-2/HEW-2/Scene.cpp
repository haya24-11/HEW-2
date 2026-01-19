#include "Scene.h"
Scene::Scene(SceneType type)
{

}

bool Scene::IsChange() const
{
    return isChange;
}

SceneType Scene::GetNextScene() const
{
    return nextScene;
}

void Scene::ChangeScene(SceneType next)
{
    isChange = true;
    nextScene = next;
}