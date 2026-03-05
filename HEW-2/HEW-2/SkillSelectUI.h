#pragma once
#include <vector>
#include "Object.h"
#include "Skill.h"

class Scene;
class GamePlay;

class SkillSelectUI
{
public:
    SkillSelectUI(Scene* scene, const std::vector<Skill*>& options);
    ~SkillSelectUI();

    void Update(float dt);
    void Uninit();

private:
    Scene* m_scene = nullptr;

    Object* m_bg = nullptr;      // 半透明背景
    Object* m_iconL = nullptr;   // 左スキルアイコン
    Object* m_iconR = nullptr;   // 右スキルアイコン
    Object* m_frameL = nullptr;  // 左選択枠
    Object* m_frameR = nullptr;  // 右選択枠

    std::vector<Skill*> m_options; // 2つのスキル
    int m_selected = 0;            // 0=左, 1=右
};