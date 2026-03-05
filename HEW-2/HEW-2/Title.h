#pragma once
#include "Scene.h"
#include <Xinput.h>

class Title : public Scene
{
public:
    Title();

    // �e�X�g����p
    int TitleMenu = 1;
    int m_count = 0;
    WORD m_prevButtons = 0;

    // �^�C�g��UI�I�u�W�F�N�g�ꗗ
    Object* TitleLogo = nullptr;
    Object* TitleTM = nullptr;
    Object* TitleBackground = nullptr;
    Object* title_backlogo = nullptr;
    Object* GameStartLogo = nullptr;
    Object* ScoreLogo = nullptr;
    Object* ExitLogo = nullptr;

    Object* TitleBackLogo = nullptr;

    float m_logoTime = 0.0f;     // �h�ꎞ��
    float m_logoCenterX = 0.0f;  // ����X�ʒu
    float m_logoBreathTime = 0.0f;

    //�V�[���̓���
    void InitScene() override;
    void UpdateScene(float deltaTime) override;
    void DrawScene() override;
    void UninitScene() override;

    // =========================
    // �^�C�g����ʁF�X���C�����s���o�p
    // =========================
    struct WalkAnim
    {
        int start = 0;       // �J�n�t���[���i0�j
        int count = 32;      // 8x4=32
        float sec = 0.10f;   // 1�t���[������
        bool loop = true;
    };

    struct SlimeWalker
    {
        Object* obj = nullptr;
        WalkAnim anim;

        int frame = 0;
        float frameTimer = 0.0f;

        float speed = 25.0f;     // px/sec
        float dir = 1.0f;        // �E=+1 / ��=-1
        float y = 0.0f;

        float baseY = 0.0f;    
        float waveAmp = 12.0f;  
        float waveSpd = 1.5f;  
        float waveT = 0.0f;    

        float baseSpeed = 20.0f;     // ����x
        float spdAmp = 6.0f;         // ���x��炬��
        float spdSpd = 1.2f;         // ���x��炬���x
        float spdT = 0.0f;           // �����^�C�}�[

        float xMin = -1200.0f;
        float xMax = 1200.0f;

        float respawnX = -1000.0f;


    };

    SlimeWalker m_green[3];
    SlimeWalker m_blue[3];
    SlimeWalker m_red[3];

private:
    static void UpdateSlimeWalker(SlimeWalker& s, float dt);
};