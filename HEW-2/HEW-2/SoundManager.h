#pragma once
#include "sound.h"

/*
    SoundManager
    =============
    ・Sound クラスのラッパー
    ・ゲーム全体のサウンド窓口
    ・SOUND_LABEL をそのまま利用する
*/

class SoundManager
{
public:
    // シングルトン取得
    static SoundManager& GetInstance();

    // 初期化（Game::Init から呼ぶ）
    void Init();
    // 終了処理（Game::Uninit から呼ぶ）
    void Uninit();
    // フェード付きBGM切り替え関数
    void PlayBGMFade(SOUND_LABEL next, float fadeTime);
    // 更新処理
    void Update(float deltaTime);

    // ---- 再生API ----

    // BGM 再生
    void Play(SOUND_LABEL label);
    // BGM 停止
    void Stop(SOUND_LABEL label);
    // SE 再生
    void PlaySE(SOUND_LABEL label);

    // サウンドの フェードイン/フェードアウト用
    void FadeInBGM(SOUND_LABEL label, float fadeTime);
    void FadeOutBGM(float fadeTime);
    // クロスフェード用
    void RequestBGM(SOUND_LABEL next);

    void SetCurrentBGM(SOUND_LABEL label);
private:
    Sound sound;   //  Sound は1インスタンスだけ持つ

    // ★ 現在再生中のBGMラベル
    // ・フェード処理
    // ・Stop / 再生制御
    // の判断基準として使う
    SOUND_LABEL m_currentBGM = SOUND_LABEL_MAX;
    SOUND_LABEL m_fadeOutBGM = SOUND_LABEL_MAX;
    SOUND_LABEL m_fadeInBGM = SOUND_LABEL_MAX;
    SOUND_LABEL m_nextBGM = SOUND_LABEL_MAX;

    SoundManager() = default;
    ~SoundManager() = default;

    // コピー禁止
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    // ===== フェード管理 =====
    float bgmVolume = 1.0f;    // 現在のBGM音量（0.0～1.0）
    float m_fadeTimer = 0.0f;       // フェード経過時間
    float m_fadeTime = 1.0f; // フェード時間（秒）
    float fadeDuration = 0.0f;  // フェード完了までの時間

    bool isFadeIn = false;
    bool isFadeOut = false;
    // クロスフェード用
    bool  m_isCrossFade = false;
};