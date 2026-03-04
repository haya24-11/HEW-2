#include "ComboManager.h"
#include "GamePlay.h"

void ComboManager::Init(GamePlay* scene)
{
    m_scene = scene;

    // 最大4桁
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
    m_comboCount = 0; // 攻撃開始でコンボリセット
    m_visible = true; // 攻撃開始時は表示
    m_timer = 0.0f;  // タイマーリセット
}

void ComboManager::AddHit()
{
    if (!m_attackActive) return;

    m_comboCount++;
    std::cout << "Combo = " << m_comboCount << std::endl; // デバッグ表示

    m_visible = true;
    m_timer = 0.0f;
    //m_timer = COMBO_VISIBLE_TIME;
    //m_attackActive = true;

    // =====================
    // ★跳ね開始
    // =====================
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
            // ★跳ねサイズ適用
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
    m_timer = COMBO_DISPLAY_TIME; // 3秒表示
}

void ComboManager::Update(float deltaTime)
{
    UpdateDraw();
    if (!m_visible) return;

    m_timer -= deltaTime;

    // =====================
    // ポップアニメ
    // =====================
    if (m_popTimer > 0.0f)
    {
        m_popTimer -= deltaTime;
        float t = m_popTimer / POP_TIME;
        // 1.5倍 → 1倍
        m_popScale = 1.0f + t * 0.5f;
    }
    else
    {
        m_popScale = 1.0f;
    }

    UpdateDraw();

    // タイマー処理
    if (!m_attackActive && m_visible)
    {
        m_timer -= deltaTime;
        if (m_timer <= 0.0f)
        {
            m_visible = false;

            if (m_comboCount > m_maxCombo) {
                m_maxCombo = m_comboCount;
            }
        }
        m_comboCount = 0; // 表示消えると同時にカウントもリセット

    }
}
