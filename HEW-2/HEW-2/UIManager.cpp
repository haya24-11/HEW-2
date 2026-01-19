#include "UIManager.h"
#include "UIBase.h"

void UIManager::SetUI(std::unique_ptr<UIBase> ui)
{
    currentUI = std::move(ui);
}

void UIManager::Update(float deltaTime)
{
    if (currentUI)
    {
        currentUI->Update(deltaTime);
    }
}

void UIManager::Draw()
{
    if (currentUI)
    {
        currentUI->Draw();
    }
}