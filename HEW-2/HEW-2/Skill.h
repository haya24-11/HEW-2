#pragma once
#include <SimpleMath.h>
class Player;





class Skill
{
private:

    //UŒ‚—Í‚ÆUŒ‚”ÍˆÍ
      int power = 10;
    DirectX::SimpleMath::Vector2 attckArea{10.0f, 10.0f};

       
       

public:
    void Apply(Player& player) ;
    void Remove(Player& player) const;
};
