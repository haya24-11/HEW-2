#include "SkillSelectUI.h"
#include "Scene.h"
#include "Player.h"
#include "Game.h"
#include "Input.h"

SkillSelectUI::SkillSelectUI(Scene* scene, const std::vector<Skill*>& options)
{
    m_scene = scene;
    m_options = options;

    // ============================
    // 半透明背景
    // ============================
    m_bg = scene->AddObject();
    m_bg->Init("ポップアップ用の背景画像のパスを入力");
    m_bg->SetUI(true);
    m_bg->SetPos(640, 360, 0);
    m_bg->SetSize(1280, 720, 0);
    m_bg->SetColor(0, 0, 0, 0.6f);

    // ============================
    // 左スキルアイコン
    // ============================
    m_iconL = scene->AddObject();
    m_iconL->Init(m_options[0]->GetIconPath());
    m_iconL->SetUI(true);
    m_iconL->SetPos(440, 360, 0);
    m_iconL->SetSize(200, 200, 0);

    // ============================
    // 右スキルアイコン
    // ============================
    m_iconR = scene->AddObject();
    m_iconR->Init(m_options[1]->GetIconPath());
    m_iconR->SetUI(true);
    m_iconR->SetPos(840, 360, 0);
    m_iconR->SetSize(200, 200, 0);

    // ============================
    // 選択枠（左）
    // ============================
    m_frameL = scene->AddObject();
    m_frameL->Init("選択できるスキルを表示する枠パス入力");
    m_frameL->SetUI(true);
    m_frameL->SetPos(440, 360, 0);
    m_frameL->SetSize(220, 220, 0);
    m_frameL->SetColor(1, 1, 0, 1);

    // ============================
    // 選択枠（右）
    // ============================
    m_frameR = scene->AddObject();
    m_frameR->Init("選択できるスキルを表示する枠パス入力");
    m_frameR->SetUI(true);
    m_frameR->SetPos(840, 360, 0);
    m_frameR->SetSize(220, 220, 0);
    m_frameR->SetColor(1, 1, 0, 1);
    m_frameR->SetActive(false); // 最初は左が選択
}

SkillSelectUI::~SkillSelectUI()
{
}

void SkillSelectUI::Update(float dt)
{
    // 左右キーで選択
    if (Input::IsKeyPressed(KEY_LEFT))
    {
        m_selected = 0;
        m_frameL->SetActive(true);
        m_frameR->SetActive(false);
    }
    if (Input::IsKeyPressed(KEY_RIGHT))
    {
        m_selected = 1;
        m_frameL->SetActive(false);
        m_frameR->SetActive(true);
    }

    // 決定
    if (Input::IsKeyPressed(KEY_ENTER))
    {
        Player* player = Game::GetInstance()->GetPlayer();
        player->ApplyAbility(m_options[m_selected]);

        Uninit(); // UI削除
        Game::GetInstance()->Resume();
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