#include "GamePlay.h"
#include "input.h"
#include "NormalEnemy.h"
#include "Camera2D.h"
#include <cmath>
static void PushOutCircle(Object* playerObj, Object* enemyObj);


GamePlay::GamePlay() : Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
    //カメラの大きさ調整
    m_camera.SetViewSize(640.0f, 320.0f); 

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
    player->SetSize(130.0f, 150.0f, 0.0f);
    player->SetPos(0.0f, 0.0f, 0.0f);
    player->SetCollisionRadius(30.0f);

    // ロジックと Object を紐づけ
    m_player->SetObject(player);

    // カメラ初期化（プレイヤー基準）
    {
        auto p = player->GetPos();
        m_camera.SetPosition({ p.x, p.y });
    }
    m_spawner.Init(this, m_player->GetObject());
    m_spawner.RegisterType<NormalEnemy>(1.0f);
  
}

void GamePlay::UpdateScene(float deltaTime)
{
    std::cout << "objects: " << objects.size() << std::endl;


    if (!m_player) return;
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    Object* playerObj = m_player->GetObject();
    if (!playerObj) return;

    const auto oldPos = playerObj->GetPos();

    m_player->Update(deltaTime);

    {
        auto p = playerObj->GetPos();
        m_camera.SetPosition({ p.x, p.y });
    }
    // ✅スポン + 敵 たち アップデート
    m_spawner.Update(deltaTime);
    // ✅プレイヤーが複数の敵に挟まれた時も抜け出すように(繰り返し分離)
    for (int iter = 0; iter < 3; ++iter)
    {
        bool pushed = false;

        for (const auto& e : m_spawner.GetEnemies())
        {
            if (!e) continue;

            Object* enemyObj = e->GetObject();
            if (!enemyObj) continue;

            if (playerObj->CheckCollision(*enemyObj))
            {
                PushOutCircle(playerObj, enemyObj);
                pushed = true;
            }
        }

        if (!pushed) break;
    }


    if (Input::GetKeyTrigger(VK_SPACE))
        SetNextScene(SceneType::Result);
}

void GamePlay::DrawScene()
{
    // ✅ カメラの表示範囲を適用（Object::Draw のプロジェクションがこれを使う）
    Object::SetViewSize(m_camera.GetViewW(), m_camera.GetViewH());

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
static void PushOutCircle(Object* aObj, Object* bObj)
{
    if (!aObj || !bObj) return;
    if (!aObj->CheckCollision(*bObj)) return;

    auto a3 = aObj->GetPos();
    auto b3 = bObj->GetPos();

    float dx = b3.x - a3.x;
    float dy = b3.y - a3.y;

    float distSq = dx * dx + dy * dy;
    float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

    // ✅ それぞれ設定した半径を使用
    float ra = aObj->GetCollisionRadius();
    float rb = bObj->GetCollisionRadius();

    float overlap = (ra + rb) - dist;
    if (overlap <= 0.0f) return;

    float nx = dx / dist;
    float ny = dy / dist;

    // お互いに半分ずつ押し合い
    float push = overlap * 0.5f;

    // 瞬間移動感防止(フレーム当たり最大密林制限)
    const float maxPush = 4.0f;   // ご希望の場合は2~10の間に調節
    if (push > maxPush) push = maxPush;

    aObj->SetPos(a3.x - nx * push, a3.y - ny * push, a3.z);
    bObj->SetPos(b3.x + nx * push, b3.y + ny * push, b3.z);
}

