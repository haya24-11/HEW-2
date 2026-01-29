#include "GamePlay.h"
#include "input.h"
#include "NormalEnemy.h"
#include "Camera2D.h"
#include <cmath>
#include "Texture.h"
//XINPUT
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

static void PushOutCircle(Object* playerObj, Object* enemyObj);


GamePlay::GamePlay() : Scene(SceneType::GamePlay)
{
}

void GamePlay::InitScene()
{
    // ===== プレイヤー関連テクスチャを事前ロード =====
    PreloadTexture(g_pDevice, "asset/Texture/player_idle.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_walk.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_light.png");
    PreloadTexture(g_pDevice, "asset/Texture/player_attack_heavy.png");


    Object dummy;
    dummy.Init("asset/Texture/player_attack_heavy.png", 1, 1);
    dummy.SetSize(1.0f, 1.0f, 0.0f);
    dummy.SetPos(100000.0f, 100000.0f, 0.0f); // 画面の外

    // 1フレームだけDrawしてGPUに完全に登録
    dummy.Draw();
    dummy.Uninit();

    //カメラの大きさ調整
    m_camera.SetViewSize(640.0f, 320.0f); 
    float zoom = 10.0f;
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
  
	std::cout << "(Debug) GamePlayScene!" << std::endl;


	// ���U���{�^��
	LightAttackButton = AddObject()
		->SetPos(300.0f, -250.0f, 0.0f)
		->SetSize(45.0f, 35.0f, 0.0f)
		->SetAngle(0.0f);
	LightAttackButton->Init("asset/lightattackbutton.png");

	// ���U���{�^��
	HeavyAttackButton = AddObject()
		->SetPos(260.0f, -220.0f, 0.0f)
		->SetSize(45.0f, 35.0f, 0.0f)
		->SetAngle(0.0f);
	HeavyAttackButton->Init("asset/heavyattackbutton.png");

	// �v���C���[�A�C�R��
	PlayerIcon = AddObject()
		->SetPos(-745.0f, 385.0f, 0.0f)
		->SetSize(100.0f, 100.0f, 0.0f)
		->SetAngle(0.0f);
	PlayerIcon->Init("asset/playericon.png");

	// �v���C���[HP�o�[
	PlayerHeartPointBar = AddObject()
		->SetPos(-580.0f, 390.0f, 0.0f)
		->SetSize(500.0f, 150.0f, 0.0f)
		->SetAngle(0.0f);
	PlayerHeartPointBar->Init("asset/playerheartpointbar.png");

	// �o�t�A�C�R��
	for (int i = 0; i < 5; i++) {
		// �I�u�W�F�N�g���ǉ�
		Object* newBuff = AddObject()
			->SetPos(-630.0f + (i * 50.0f), 300.0f, 0.0f) // ����50�����炵�Ĕz�u������
			->SetSize(50.0f, 50.0f, 1.0f)
			->SetAngle(0.0f);

		// �摜�̏�����
		newBuff->Init("asset/bufficon.png");

		// vector�ɒǉ����ĕێ����Ă���
		BuffIcons.push_back(newBuff);
	}

	// ���@�w
	MagicCircle = AddObject()
		->SetPos(800.0f, 450.0f, 0.0f)
		->SetSize(400.0f, 400.0f, 0.0f)
		->SetAngle(0.0f);
	MagicCircle->Init("asset/magiccircle.png");

	/* �GHP�o�[
	EnemyHeartPointBar = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	EnemyHeartPointBar->Init("asset/enemyheartpointbar.png");
	*/
	/*
	// �o���l�o�[
	ExpBar = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	ExpBar->Init("asset/expbar.png");
	*/
	/*
	// �R���{�\��
	Combo = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Combo->Init("asset/combo.png");
	*/
	/*
	// �v���C���[
	Player = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Player->Init("asset/player.png");
	*/
	/*
	// �G
	Enemy = AddObject()
		->SetPos(0.0f, 0.0f, 0.0f)
		->SetSize(0.0f, 0.0f, 0.0f)
		->SetAngle(0.0f);
	Enemy->Init("asset/enemy.png");
	*/
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

    m_spawner.Update(deltaTime);
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

    //PAD処理シ－ン切り替え
    static WORD prevButtons = 0;

    XINPUT_STATE pad{};
    WORD buttons = 0;
    if (XInputGetState(0, &pad) == ERROR_SUCCESS)
        buttons = pad.Gamepad.wButtons;

    auto PadTrigger = [&](WORD mask) -> bool
        {
            return (buttons & mask) && !(prevButtons & mask);
        };

    if (Input::GetKeyTrigger(VK_SPACE) || PadTrigger(XINPUT_GAMEPAD_RIGHT_SHOULDER))
    {
        SetNextScene(SceneType::Result);
    }

    prevButtons = buttons;
}
void GamePlay::DrawScene()
{
    auto off = m_camera.GetOffset();

    for (auto& obj : objects)
    {
        auto old = obj->GetPos();
        obj->SetPos(old.x + off.x, old.y + off.y, old.z);

        int frame = -1;

        if (obj.get() == m_player->GetObject())
        {
            frame = m_player->GetAnimFrame();
        }
        else
        {
            for (const auto& e : m_spawner.GetEnemies())
            {
                if (!e) continue;
                if (e->GetObject() == obj.get())
                {
                    frame = e->GetAnimFrame();
                    break;
                }
            }
        }

        if (frame >= 0) obj->Draw(frame);
        else            obj->Draw();

        obj->SetPos(old.x, old.y, old.z);
    }
}


void GamePlay::UninitScene()
{
    Object::ReleaseTextureCache();
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

    // 実際の当たり判定半径を使用（Object に GetCollisionRadius() が必要）
    float rp = playerObj->GetCollisionRadius();
    float re = enemyObj->GetCollisionRadius();

    // 距離がほぼ0のとき（法線が作れず押し出しが暴れるのを防ぐ）
    float dist = (distSq > 0.0001f) ? sqrtf(distSq) : 0.0f;
    if (dist <= 0.0001f)
    {
        // 完全に重なっている場合：適当な方向に少しだけずらして抜け出す（瞬間移動っぽさ防止）
        playerObj->SetPos(p3.x + 1.0f, p3.y, p3.z);
        return;
    }

    // 重なり量（半径合計 - 実距離）
    float overlap = (rp + re) - dist;
    if (overlap <= 0.0f) return;

    // 押し出し方向（正規化ベクトル）
    float nx = dx / dist;
    float ny = dy / dist;

    // 1フレームで押し出す最大量を制限（大きく飛ぶ＝瞬間移動を防ぐ）
    const float maxPushPerFrame = 10.0f; 
    float push = overlap;
    if (push > maxPushPerFrame) push = maxPushPerFrame;

    // プレイヤーだけを押し出す（敵は固定）
    playerObj->SetPos(p3.x + nx * push, p3.y + ny * push, p3.z);
}
