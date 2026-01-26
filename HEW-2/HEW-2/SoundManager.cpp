#include "SoundManager.h"
#include <stdio.h>

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

void SoundManager::PlayBGMFade(SOUND_LABEL next, float fadeTime)
{
    if (next == SOUND_LABEL_MAX)
        return;

    m_fadeOutBGM = m_currentBGM;
    m_fadeInBGM = next;

    m_fadeTimer = 0.0f;
    m_fadeTime = fadeTime;

    if (m_fadeInBGM != SOUND_LABEL_MAX)
    {
        Play(m_fadeInBGM);
        sound.SetVolume(m_fadeInBGM, 0.0f);
    }

    m_currentBGM = next;
}

void SoundManager::Update(float deltaTime)
{
    if (!m_isCrossFade) return;
    if (m_fadeTime <= 0.0f) return;

    m_fadeTimer += deltaTime;
    float t = m_fadeTimer / m_fadeTime;
    if (t > 1.0f) t = 1.0f;

    printf(
        "[Sound] current=%d next=%d fadeOut=%d t=%.2f\n",
        m_currentBGM,
        m_nextBGM,
        m_fadeOutBGM,
        t
    );

    // フェードアウト（旧BGM）
    if (m_fadeOutBGM != SOUND_LABEL_MAX)
    {
        sound.SetVolume(m_fadeOutBGM, 1.0f - t);

        if (t >= 1.0f)
        {
            sound.Stop(m_fadeOutBGM);
            m_fadeOutBGM = SOUND_LABEL_MAX;
        }
    }

    // フェードイン（新BGM）
    if (m_nextBGM != SOUND_LABEL_MAX)
    {
        sound.SetVolume(m_nextBGM, t);
    }

    // クロスフェード完了
    if (t >= 1.0f)
    {
        m_currentBGM = m_nextBGM;
        m_nextBGM = SOUND_LABEL_MAX;
        m_isCrossFade = false;
    }
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

void SoundManager::FadeInBGM(SOUND_LABEL label, float fadeTime)
{
    // すでに同じBGMなら無視
    if (m_currentBGM == label) return;

    m_currentBGM = label;

    fadeDuration = fadeTime;
    m_fadeTimer = 0.0f;

    bgmVolume = 0.0f;

    sound.Play(label);
    sound.SetVolume(label, 0.0f);

    isFadeIn = true;
    isFadeOut = false;
}

void SoundManager::FadeOutBGM(float fadeTime)
{
    if (m_currentBGM == SOUND_LABEL_MAX) return;

    fadeDuration = fadeTime;
    m_fadeTimer = 0.0f;

    isFadeOut = true;
    isFadeIn = false;
}

void SoundManager::RequestBGM(SOUND_LABEL next)
{
    if (next == m_currentBGM)
        return; // 同じBGMなら何もしない

    m_nextBGM = next;
    m_fadeTimer = 0.0f;
    m_isCrossFade = true;

    // 新BGMを無音で再生開始
    sound.Play(m_nextBGM);
    sound.SetVolume(m_nextBGM, 0.0f);
}

void SoundManager::SetCurrentBGM(SOUND_LABEL label)
{
    m_currentBGM = label;
}