#include "Scene.h"

Scene::Scene(SceneType type):sceneType(type)
{

}

Scene::~Scene()
{
	ClearObject();
}

void Scene::InitScene()
{

}

void Scene::UpdateScene(float deltaTime)
{

}

void Scene::DrawScene()
{

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

void Scene::ChangeScene(SceneType next)
{
    isChange = true;
    nextScene = next;
}

void Scene::SetNextScene(SceneType nextScene)
{
	// ç°Ç∆ìØÇ∂ÉVÅ[ÉìÇ»ÇÁèàóùÇ»Çµ
	if (sceneType == nextScene)
		return;

	Scene::nextScene = nextScene;
}

Object* Scene::AddObject()
{
	objects.push_back(std::make_unique<Object>());
	return objects.back().get();
}

void Scene::ClearObject()
{
	objects.clear();
	objects.shrink_to_fit();
}
