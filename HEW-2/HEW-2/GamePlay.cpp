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
    // 当たり判定範囲
    player->SetCollisionRadius(50.0f);

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

    // ✅ 스폰 + 적들 업데이트
    m_spawner.Update(deltaTime);
    // ✅ 플레이어가 여러 적에 끼었을 때도 빠져나오도록(반복 분리)
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

static void PushOutCircle(Object* playerObj, Object* enemyObj)
{
    if (!playerObj || !enemyObj) return;
    if (!playerObj->CheckCollision(*enemyObj)) return;

    auto p3 = playerObj->GetPos();
    auto e3 = enemyObj->GetPos();

    float dx = p3.x - e3.x;
    float dy = p3.y - e3.y;

    float distSq = dx * dx + dy * dy;
    float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.01f;

    // ✅ 반지름 getter 없으니 우선 하드코딩(플레이어 50, 적 50)
    // 가능하면 Object에 GetCollisionRadius() 만들어서 그 값 쓰는 게 정석
    float rp = 50.0f;
    float re = 50.0f;

    float overlap = (rp + re) - dist;
    if (overlap <= 0.0f) return;

    float nx = dx / dist;
    float ny = dy / dist;

    // 플레이어만 밀어내기(적은 그대로)
    float push = overlap + 0.5f; // 살짝 여유를 줘서 끼임 방지
    playerObj->SetPos(p3.x + nx * push, p3.y + ny * push, p3.z);
}
