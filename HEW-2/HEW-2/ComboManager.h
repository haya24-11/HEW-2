#pragma once
#include <vector>
#include "Object.h"
#include <string>
#include <iostream>

class GamePlay;

class ComboManager
{
public:
    void Init(GamePlay* scene);
    void Update(float deltaTime);

    void BeginAttack();   // 攻撃開始でリセット
    void AddHit();        // ヒット加算
    void UpdateUI();

private:
    GamePlay* m_scene = nullptr;
    Object* m_debugText = nullptr;

    int   m_comboCount = 0;
    bool  m_attackActive = false;
    bool  m_visible = false;

    float m_timer = 0.0f;
    const float COMBO_VISIBLE_TIME = 1.0f;

    std::vector<Object*> m_digits;
};