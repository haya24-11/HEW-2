#pragma once
#include <SimpleMath.h>
#include <string>

class Player;

/*
    Skill
    =====
    ・スキルの最小単位
    ・効果の内容は派生クラスで定義
*/

class Skill
{
public:
    Skill();
    virtual ~Skill() = default;

     
    virtual void Apply(Player* player) = 0;
    virtual void Remove(Player* player) = 0;

protected:
    std::string skillName;
    int skillId = 0;
};
