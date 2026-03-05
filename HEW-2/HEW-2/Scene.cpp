#include "Scene.h"

Scene::Scene(SceneType type) : sceneType(type)
{
}

Scene::~Scene()
{
    ClearObject();
}

void Scene::InitScene()
{
}

void Scene::UpdateScene(float /*deltaTime*/)
{
}

void Scene::DrawScene()
{
    for (auto& obj : objects)
        obj->Draw();
}

void Scene::UninitScene()
{
}

void Scene::CommonInit()
{
    nextScene = SceneType::NONE;
    InitScene();
}

bool Scene::IsChange() const
{
    return isChange;
}

const SceneType Scene::GetNextScene() const
{
    return nextScene;
}

void Scene::ChangeScene(SceneType next, const ResultData& data)
{
    m_lastResult = data;
    isChange = true;
    nextScene = next;
}

void Scene::SetNextScene(SceneType nextScene)
{
    // 今と同じシーンなら処理なし
    if (sceneType == nextScene)
        return;

    Scene::nextScene = nextScene;
}

Object* Scene::AddObject()
{
    objects.push_back(std::make_unique<Object>());
    return objects.back().get();
}

void Scene::RemoveObject(Object* obj)
{
    if (!obj) return;

    // ✅ 先に Object 側の GPU リソースを解放してから削除
    for (auto it = objects.begin(); it != objects.end(); )
    {
        if (it->get() == obj)
        {
            if (*it) (*it)->Uninit();
            it = objects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Scene::ClearObject()
{
    // ✅ clear前に全ObjectのUninitを呼んでGPU/参照カウントを解放
    for (auto& o : objects)
    {
        if (o) o->Uninit();
    }

    objects.clear();
    objects.shrink_to_fit();
}

