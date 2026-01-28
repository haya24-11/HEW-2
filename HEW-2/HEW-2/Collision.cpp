#include "Collision.h"

#include <cmath>

void Collision::SetPosition(float x, float y)
{
    m_pos.x = x;
    m_pos.y = y;
}

void Collision::SetPosition(const DirectX::XMFLOAT2& pos)
{
    m_pos = pos;
}

const DirectX::XMFLOAT2& Collision::GetPosition() const
{
    return m_pos;
}

void Collision::SetRadius(float r)
{
    m_radius = (r < 0.0f) ? 0.0f : r;
}

float Collision::GetRadius() const
{
    return m_radius;
}

bool Collision::Intersects(const Collision& other) const
{
    return Intersects(*this, other);
}

bool Collision::Intersects(const Collision& a, const Collision& b)
{
    const float dx = a.m_pos.x - b.m_pos.x;
    const float dy = a.m_pos.y - b.m_pos.y;
    const float rr = a.m_radius + b.m_radius;

    // ‹——£^2 <= (r1+r2)^2
    return (dx * dx + dy * dy) <= (rr * rr);
}
