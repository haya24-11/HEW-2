#include "SoundManager.h"

/*
    SoundManager.cpp
    ----------------
    ・Sound クラスを内部で保持
    ・再生用途ごとに API を分ける
*/

SoundManager& SoundManager::GetInstance()
{
    static SoundManager instance;
    return instance;
}

void SoundManager::Init()
{
    // Sound の初期化
    sound.Init();
}

void SoundManager::Uninit()
{
    sound.Uninit();
}

void SoundManager::Play(SOUND_LABEL label)
{
    // BGM用途の再生
    sound.Play(label);
}

void SoundManager::Stop(SOUND_LABEL label)
{
    sound.Stop(label);
}

void SoundManager::PlaySE(SOUND_LABEL label)
{
    // SEは多重再生される想定だが
    // 今の Sound 実装では「上書き再生」になる
    sound.Play(label);
}