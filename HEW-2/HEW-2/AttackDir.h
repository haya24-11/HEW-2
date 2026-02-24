#pragma once

// ===============================
// çUåÇï˚å¸ ã§í íËã`
// ===============================
enum class AttackDir
{
    Right,
    Left,
    Up,
    Down,

    UpRight,
    UpLeft,
    DownRight,
    DownLeft
};

// =======================================
// ã§í ÅFAttackDir Å® Vectorïœä∑
// =======================================
inline DirectX::SimpleMath::Vector2
AttackDirToVector(AttackDir dir)
{
    using namespace DirectX::SimpleMath;

    switch (dir)
    {
    case AttackDir::Right: return { 1,0 };
    case AttackDir::Left:  return { -1,0 };
    case AttackDir::Up:    return { 0,1 };
    case AttackDir::Down:  return { 0,-1 };

    case AttackDir::UpRight:    return { 0.707f,0.707f };
    case AttackDir::UpLeft:     return { -0.707f,0.707f };
    case AttackDir::DownRight:  return { 0.707f,-0.707f };
    case AttackDir::DownLeft:   return { -0.707f,-0.707f };
    }

    return { 1,0 };
}