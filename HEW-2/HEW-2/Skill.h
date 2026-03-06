#pragma once
#include <string>
#include <SimpleMath.h>

class Player;

class Skill
{
public:
    virtual ~Skill() = default;

    virtual void Apply(Player* player) = 0;
    virtual void Remove(Player* player) = 0;

    int GetId() const;
    const std::string& GetName() const;

    //スキルアイコンのパスのゲッター
    virtual const char* GetIconPath() const = 0;
    int GetLevel() const { return m_skilllLevel; }
protected:
    std::string skillName;
    int skillId = 0;
    int m_skilllLevel = 0;
};

