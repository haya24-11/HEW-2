#include "ComboManager.h"
#include "GamePlay.h"

static constexpr float COMBO_DISPLAY_TIME = 3.0f;

void ComboManager::Init(GamePlay* scene)
{
    m_scene = scene;

    // ✅ リプレイ対策：前回のポインタを必ず破棄（ClearObject 後に触ると危険）
    m_digits.clear();
    m_debugText = nullptr;

    // 状態を初期化
    m_comboCount = 0;
    m_attackActive = false;
    m_visible = false;
    m_timer = 0.0f;
    m_popScale = 1.0f;
    m_popTimer = 0.0f;

    if (!m_scene) return;

    // 4桁分のUIを生成
    m_digits.reserve(4);
    for (int i = 0; i < 4; i++)
    {
        Object* digit = m_scene->AddObject();
        if (!digit) continue;

        digit->Init("asset/UI/combo_number.png", 10, 1);
        digit->SetSpriteSheet(10, 1);
        digit->SetUI(true);
        digit->SetSize(64, 64, 0);

        // 最初は非表示
        digit->SetActive(false);

        m_digits.push_back(digit);
    }
}

void ComboManager::BeginAttack()
{
    m_attackActive = true;
    m_comboCount = 0; 
    m_visible = false; 
    m_timer = 0.0f;  
}

void ComboManager::AddHit()
{
    if (!m_attackActive) return;

    m_comboCount++;
    std::cout << "Combo = " << m_comboCount << std::endl; 

    m_visible = true;
    m_timer = COMBO_DISPLAY_TIME;
    m_popTimer = POP_TIME;
}

void ComboManager::UpdateDraw()
{
    for (auto d : m_digits)
        if (d) d->SetActive(false);

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

void ComboManager::AddCombo(int value)
{
    m_comboCount += value;

    if (m_comboCount > m_maxCombo)
        m_maxCombo = m_comboCount;

    m_visible = true;
    m_timer = COMBO_DISPLAY_TIME;

    std::cout << "[DEBUG] Combo = " << m_comboCount << std::endl;
}

void ComboManager::Update(float deltaTime)
{

    if (m_popTimer > 0.0f)
    {
        m_popTimer -= deltaTime;
        float t = m_popTimer / POP_TIME;
  
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

            

            for (auto d : m_digits)
                d->SetActive(false);

            return;
        }
        //m_comboCount = 0;

    }

    UpdateDraw();
}
