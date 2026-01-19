#pragma once
#include <memory>      // std::unique_ptr

class UIBase;          // 前方宣言（include不要）

class UIManager
{
public:
    void SetUI(std::unique_ptr<UIBase> ui);
    void Update(float deltaTime);
    void Draw();

private:
    std::unique_ptr<UIBase> currentUI;
};