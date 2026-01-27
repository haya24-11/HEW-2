#include "GamePlay.h"
#include "input.h"
#include "NormalEnemy.h"
#include "Camera2D.h"

GamePlay::GamePlay() : Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
    std::cout << "InitScene" << std::endl;

    // MAP
    Object* map = AddObject();
    map->Init("asset/Texture/map.png");
    map->SetPos(0.0f, 0.0f, 0.0f);
    map->SetSize(3000.0f, 3000.0f, 0.0f);

    m_player = std::make_unique<Player>();

    Object* player = AddObject();
    player->Init("asset/Texture/player_idle.png");
    player->SetSpriteSheet(6, 6);
    player->SetSize(150.0f, 170.0f, 0.0f);
    player->SetPos(0.0f, 0.0f, 0.0f);
    //当たり判定範囲
    player->SetCollisionRadius(50.0f);

    //로직과 Object 연결
    m_player->SetObject(player);

    // camera 초기화(플레이어 기준)
    {
        auto p = player->GetPos();
        m_camera.SetPosition({ p.x, p.y });
    }

    // ENEMY OBJECT
    Object* nenemy = AddObject();
    nenemy->Init("asset/Texture/NormalEnemy.png");
    nenemy->SetPos(200.0f, 0.0f, 0.0f);
    nenemy->SetSize(100.0f, 100.0f, 0.0f);
    nenemy->SetCollisionRadius(50.0f);

    // enemy 로직
    m_enemy = std::make_unique<NormalEnemy>();
    m_enemy->SetObject(nenemy);
    m_enemy->SetTarget(m_player->GetObject());
}

void GamePlay::UpdateScene(float deltaTime)
{
    if (!m_player) return;
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    // プレーヤーObject
    Object* playerObj = m_player->GetObject();
    if (!playerObj) return;

    // 敵Object（m_enemy が存在する場合はそこから取得）
    Object* enemyObj = nullptr;
    if (m_enemy)
        enemyObj = m_enemy->GetObject();  //（Enemy / Chara に GetObject があればOK）

    // プレイヤー移動前の位置保存
    const auto oldPos = playerObj->GetPos();

    // プレイヤーアップデート（入力/移動）
    m_player->Update(deltaTime);

    // カメラをプレイヤーに追従させる（移動後の最新座標を反映）
    {
        auto p = playerObj->GetPos();
        m_camera.SetPosition({ p.x, p.y });
    }

    // プレーヤーと敵が衝突すればプレーヤーを戻す
    if (enemyObj && playerObj->CheckCollision(*enemyObj))
    {
        playerObj->SetPos(oldPos.x, oldPos.y, oldPos.z);
    }

    // 敵アップデート（プレイヤー位置基準追跡）
    if (m_enemy)
        m_enemy->Update(deltaTime);

    // 画面遷移
    if (Input::GetKeyTrigger(VK_SPACE))
        SetNextScene(SceneType::Result);
}

void GamePlay::DrawScene()
{
    auto off = m_camera.GetOffset();

    for (auto& obj : objects)
    {
        auto old = obj->GetPos();

        // カメラオフセットを適用（描画中のみ一時的に座標をずらす）
        obj->SetPos(old.x + off.x, old.y + off.y, old.z);

        if (obj.get() == m_player->GetObject())
        {
            obj->Draw(m_player->GetAnimFrame());
        }
        else
        {
            obj->Draw();
        }

        // 元の位置に戻す
        obj->SetPos(old.x, old.y, old.z);
    }
}

void GamePlay::UninitScene()
{
    std::cout << "UninitScene" << std::endl;
}
