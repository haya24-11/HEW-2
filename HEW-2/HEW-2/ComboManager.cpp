#include "ComboManager.h"
#include "GamePlay.h"

static constexpr float COMBO_DISPLAY_TIME = 3.0f;

void ComboManager::Init(GamePlay* scene)
{
    m_scene = scene;

    // �ő�4��
    for (int i = 0; i < 4; i++)
    {
        Object* digit = m_scene->AddObject();
        digit->Init("asset/UI/combo_number.png", 10, 1);
        digit->SetSpriteSheet(10, 1);
        digit->SetUI(true);
        digit->SetSize(64, 64, 0);

        m_digits.push_back(digit);
    }
    for (auto d : m_digits)
    {
        d->SetActive(false);
    }
}

void ComboManager::BeginAttack()
{
    m_attackActive = true;
    m_comboCount = 0; // �U���J�n�ŃR���{���Z�b�g
    m_visible = false; // �U���J�n���͕\��
    m_timer = 0.0f;  // �^�C�}�[���Z�b�g
}

void ComboManager::AddHit()
{
    if (!m_attackActive) return;

    m_comboCount++;
    std::cout << "Combo = " << m_comboCount << std::endl; // �f�o�b�O�\��

    m_visible = true;
    m_timer = COMBO_DISPLAY_TIME;
    m_popTimer = POP_TIME;
}

void ComboManager::UpdateDraw()
{
    for (auto d : m_digits)
        d->SetActive(false);

    if (!m_visible) return;

    float baseX = SCREEN_WIDTH * 0.5f - 120.0f;
    float baseY = SCREEN_HEIGHT * 0.5f - 60.0f;

    int value = m_comboCount;

    int index = 0;

    do
    {
        int num = value % 10;

        if (index < m_digits.size())
        {
            auto obj = m_digits[index];
            obj->SetActive(true);
            obj->SetAnimFrame(num);

            // =====================
            // �����˃T�C�Y�K�p
            // =====================
            obj->SetSize(
                64.0f * m_popScale,
                64.0f * m_popScale,
                0.0f
            );
            obj->SetPos(
                baseX - index * 70.0f,
                baseY,
                0.0f
            );
        }

        value /= 10;
        index++;

    } while (value > 0);
}

void ComboManager::EndAttack()
{
    m_attackActive = false;
    m_timer = COMBO_DISPLAY_TIME; // 3�b�\��
}

void ComboManager::Update(float deltaTime)
{

    // =====================
    // �|�b�v�A�j��
    // =====================
    if (m_popTimer > 0.0f)
    {
        m_popTimer -= deltaTime;
        float t = m_popTimer / POP_TIME;
        // 1.5�{ �� 1�{
        m_popScale = 1.0f + t * 0.5f;
    }
    else
    {
        m_popScale = 1.0f;
    }

    if (m_visible)
    {
        m_timer -= deltaTime;
        if (m_timer <= 0.0f)
        {
            m_visible = false;

            m_comboCount = 0;

            for (auto d : m_digits)
                d->SetActive(false);

            return;
        }
        m_comboCount = 0; // �\��������Ɠ����ɃJ�E���g�����Z�b�g

    }

    UpdateDraw();
}
