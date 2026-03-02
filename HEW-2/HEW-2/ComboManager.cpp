#include "ComboManager.h"
#include "GamePlay.h"

void ComboManager::Init(GamePlay* scene)
{
    m_scene = scene;

    // ç≈ëÂ4åÖ
    for (int i = 0; i < 4; i++)
    {
        Object* digit = m_scene->AddObject();
        digit->Init("asset/UI/combo_number.png", 10, 1);
        digit->SetSpriteSheet(10, 1);
        digit->SetUI(true);
        digit->SetSize(64, 64, 0);

        m_digits.push_back(digit);
    }
    m_digits[0]->SetPos(0, 0, 0);
    m_digits[0]->SetSize(300, 300, 0);
    m_digits[0]->SetColor(1, 0, 0, 1);
}

void ComboManager::BeginAttack()
{
    m_comboCount = 0;
    m_attackActive = true;
}

void ComboManager::AddHit()
{
    if (!m_attackActive) return;

    m_comboCount++;

    std::cout << "Combo = "
        << m_comboCount
        << std::endl;

    m_visible = true;
    m_timer = COMBO_VISIBLE_TIME;
    m_attackActive = true;
}

void ComboManager::UpdateUI()
{
    if (!m_visible) return;

    float halfW = SCREEN_WIDTH * 0.5f;
    float halfH = SCREEN_HEIGHT * 0.5f;

    int value = m_comboCount;

    int index = 0;

    do
    {
        int num = value % 10;

        if (index < m_digits.size())
        {
            auto obj = m_digits[index];

            obj->SetAnimFrame(num);

            obj->SetPos(
                halfW - 80.0f - index * 70.0f,
                halfH - 80.0f,
                0.0f
            );
        }

        value /= 10;
        index++;

    } while (value > 0);
}

void ComboManager::Update(float deltaTime)
{
    if (!m_visible) return;

    m_timer -= deltaTime;

    if (m_timer <= 0.0f)
    {
        m_visible = false;
        m_attackActive = false;
        m_comboCount = 0;
    }
}

