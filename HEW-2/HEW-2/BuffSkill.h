#pragma once
#include "Skill.h"
class BuffSkill :
    public Skill
{
public:
    BuffSkill(const std::string& name, int id);
    void Apply(Player* player) override;
    void Remove(Player* player) override;
    int GetSkilllevel() { return m_skillbufflevel; };
protected:

  
};

