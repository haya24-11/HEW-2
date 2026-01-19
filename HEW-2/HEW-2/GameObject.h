#pragma once
#include "MathTypes.h"

class GameObject
{
public:
    virtual ~GameObject() = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;

    Vector2 GetPosition() const;
    void SetPosition(const Vector2& pos);

protected:
    Vector2 position;
};