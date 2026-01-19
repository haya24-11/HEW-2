#pragma once

class Player;

class Skill
{
public:
    void Apply(Player& player) const;
    void Remove(Player& player) const;
};
