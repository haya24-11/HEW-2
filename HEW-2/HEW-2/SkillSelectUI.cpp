#include "SkillSelectUI.h"
#include "Scene.h"
#include "GamePlay.h"
#include "Player.h"
#include "Input.h"

SkillSelectUI::SkillSelectUI(Scene* scene, const std::vector<Skill*>& options)
{
    m_scene = scene;
    m_options = options;

    // ============================
    // 半透明背景
    // ============================
    m_bg = scene->AddObject();
    m_bg->Init("asset/123.png");
    m_bg->SetUI(true);
    m_bg->SetPos(0, 0, 0);
    m_bg->SetSize(1280, 720, 0);

    // ============================
    // 左スキルアイコン
    // ============================
    m_iconL = scene->AddObject();
    m_iconL->Init(m_options[0]->GetIconPath());
    m_iconL->SetUI(true);
    m_iconL->SetPos(-220, 0, 0);
    m_iconL->SetSize(200, 200, 0);

    // ============================
    // 右スキルアイコン
    // ============================
    m_iconR = scene->AddObject();
    m_iconR->Init(m_options[1]->GetIconPath());
    m_iconR->SetUI(true);
    m_iconR->SetPos(220, 0, 0);
    m_iconR->SetSize(200, 200, 0);

    // ============================
    // 選択枠（左）
    // ============================
    m_frameL = scene->AddObject();
    m_frameL->Init("asset/UI/bufficon.png");
    m_frameL->SetUI(true);
    m_frameL->SetPos(-220, 0, 0);
    m_frameL->SetSize(220, 220, 0);
    m_frameL->SetColor(1, 1, 0, 1);

    // ============================
    // 選択枠（右）
    // ============================
    m_frameR = scene->AddObject();
    m_frameR->Init("asset/UI/bufficon.png");
    m_frameR->SetUI(true);
    m_frameR->SetPos(220, 0, 0);
    m_frameR->SetSize(220, 220, 0);
    m_frameR->SetColor(1, 1, 0, 1);
    m_frameR->SetActive(false); // 最初は左が選択
}

SkillSelectUI::~SkillSelectUI()
{
    Uninit();
}

void SkillSelectUI::Update(float dt)
{
    // ============================
    // 左右キーで選択
    // ============================

    // キーボード
    if (Input::GetKeyTrigger(VK_LEFT))
    {
        m_selected = 0;
        m_frameL->SetActive(true);
        m_frameR->SetActive(false);
    }
    if (Input::GetKeyTrigger(VK_RIGHT))
    {
        m_selected = 1;
        m_frameL->SetActive(false);
        m_frameR->SetActive(true);
    }

    // コントローラー
    if (Input::GetButtonTrigger(XINPUT_LEFT))
    {
        m_selected = 0;
        m_frameL->SetActive(true);
        m_frameR->SetActive(false);
    }
    if (Input::GetButtonTrigger(XINPUT_RIGHT))
    {
        m_selected = 1;
        m_frameL->SetActive(false);
        m_frameR->SetActive(true);
    }

    // ============================
    // 決定（Enter or Aボタン）
    // ============================
    if (Input::GetKeyTrigger(VK_RETURN) || Input::GetButtonTrigger(XINPUT_A))
    {
        GamePlay* gameplay = dynamic_cast<GamePlay*>(m_scene);
        if (!gameplay) return;

        Player* player = gameplay->GetPlayer();
        if (!player) return;

        // 選ばれたスキルを適用
        player->ApplyAbility(m_options[m_selected]);

        // UI削除 & ゲーム再開
        Uninit();
        gameplay->Resume();
        gameplay->ClearSkillUI();
    }
}

void SkillSelectUI::Uninit()
{
    if (!m_scene) return;

    m_scene->RemoveObject(m_bg);
    m_scene->RemoveObject(m_iconL);
    m_scene->RemoveObject(m_iconR);
    m_scene->RemoveObject(m_frameL);
    m_scene->RemoveObject(m_frameR);

    m_bg = m_iconL = m_iconR = m_frameL = m_frameR = nullptr;
}
