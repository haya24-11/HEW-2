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
    void UpdateDraw();
    void EndAttack(); // 攻撃終了時に呼ぶ

    int GetMaxCombo() const { return m_maxCombo; } // Result掲載用
private:
    GamePlay* m_scene = nullptr;
    Object* m_debugText = nullptr;

    int   m_comboCount = 0;
    int   m_maxCombo = 0;
    bool  m_attackActive = false;
    bool  m_visible = false;

    float m_timer = 0.0f;
    const float COMBO_VISIBLE_TIME = 1.0f;
    // =====================
    // コンボポップ演出用
    // =====================
    float m_popScale = 1.0f;
    float m_popTimer = 0.0f;
    const float POP_TIME = 0.15f;

    const float COMBO_DISPLAY_TIME = 3.0f; // 攻撃停止後表示する時間

    std::vector<Object*> m_digits;
};
