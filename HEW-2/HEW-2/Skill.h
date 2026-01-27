#pragma once
#include <SimpleMath.h>
#include <string>
#include"SkillType.h"

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
    Skill(const std::string& name, int id,SkillType skilltype);
    virtual ~Skill() = default;

     
    virtual void Apply(Player* player) = 0;
    virtual void Remove(Player* player) = 0;

protected:
    std::string skillName;
    int skillId = 0;
    SkillType skillType;


};
