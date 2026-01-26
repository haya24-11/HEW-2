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
private:
    Sound sound;   //  Sound は1インスタンスだけ持つ

    SoundManager() = default;
    ~SoundManager() = default;

    // コピー禁止
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;
public:
    // シングルトン取得
    static SoundManager& GetInstance();

    // 初期化（Game::Init から呼ぶ）
    void Init();

    // 終了処理（Game::Uninit から呼ぶ）
    void Uninit();

    // ---- 再生API ----

    // BGM 再生
    void Play(SOUND_LABEL label);

    // BGM 停止
    void Stop(SOUND_LABEL label);

    // SE 再生
    void PlaySE(SOUND_LABEL label);


};