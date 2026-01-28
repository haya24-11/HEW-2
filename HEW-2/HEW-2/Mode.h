#pragma once
#include <vector>
#include <string>
#include "Skill.h"
#include "Combo.h"
#include "Exp.h"

class Mode
{
public:
    const std::vector<Skill>& GetSkills() const;

protected:
    std::string name;
    std::vector<Skill> skills;
};
