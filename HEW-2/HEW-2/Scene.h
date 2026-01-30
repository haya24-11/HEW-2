#pragma once
#include <vector>
#include <memory>
#include "SceneTypes.h"
#include "input.h"
#include "Object.h"
#include<iostream>

class Scene
{
private:
    SceneType nextScene = SceneType::NONE;
    SceneType sceneType;
protected:
    void ChangeScene(SceneType next);



    std::vector<std::unique_ptr<Object>> objects;//オブジェクトの格納配列
    int objectNum = 0;//オブジェクトの数
    bool isChange = false;
    void SetNextScene(SceneType nextScene);
    virtual void InitScene() = 0;

public:
    Scene(SceneType type);//なんのシーンかを名義する
    virtual ~Scene();

    virtual void UpdateScene(float deltaTime) = 0;
    virtual void DrawScene() = 0;
    virtual void UninitScene() = 0;

    void CommonInit();//全シーン共通初期化

    bool IsChange() const;
    const SceneType GetNextScene() const;//次のシーンを取得
    Object* AddObject();
    void RemoveObject(Object* obj);
    void ClearObject();
};
