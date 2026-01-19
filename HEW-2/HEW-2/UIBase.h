#pragma once
class UIBase
{
public:
    virtual ~UIBase() = default;

    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
};

