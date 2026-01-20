#pragma once
#include "Mode.h"

class AMode : public Mode
{
    // Mode‚²‚Æ‚Ì“Áêƒ‹[ƒ‹
      //UŒ‚—Í‚ÆUŒ‚”ÍˆÍ
    int weakPower = 10;
    int strongPower = 50;

    DirectX::SimpleMath::Vector2 attckArea{ 10.0f, 10.0f };

    void WeakAttck();
    void StrongAttkc();

};