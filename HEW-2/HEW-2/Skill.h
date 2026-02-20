#pragma once
#include <string>
#include <SimpleMath.h>
#include "AMode.h"
#include "Player.h"

class Player;

class Skill
{
public:
    virtual ~Skill() = default;

    virtual void Apply(Player* player) = 0;
    virtual void Remove(Player* player) = 0;

    int GetId() const;
    const std::string& GetName() const;

protected:
    std::string skillName;
    int skillId = 0;
};

