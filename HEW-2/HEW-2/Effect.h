#pragma once
class Scene;
class Object;

class Effect
{
public:
    virtual ~Effect() = default;

    virtual void Update(float dt) = 0;
    virtual bool IsDead() const = 0;
    virtual void Uninit() {}

protected:
    Scene* m_scene = nullptr;
    Object* m_object = nullptr;
};