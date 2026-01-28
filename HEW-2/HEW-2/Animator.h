#pragma once

/*
    Animation
    =========
    ・スプライトシート上のアニメーション定義
    ・どのフレームから何枚
    ・1フレームの再生時間
*/
struct Animation
{
    int startFrame;         // 開始フレーム
    int frameCount;       // フレーム数
    float frameTime;      // 1フレームの秒数
    bool loop;                // ループするか
};

/*
    Animator
    ========
    ・アニメーションの時間管理専用
    ・描画や入力には一切関与しない
*/
class Animator
{
public:
    // アニメーション再生
    void Play(const Animation& anim);
    // 毎フレーム更新
    void Update(float deltaTime);

    // 現在のフレーム番号を取得（スプライト用）
    int GetCurrentFrame() const;

    // 再生中かどうか
    bool IsPlaying() const { return m_currentAnim != nullptr; }
private:
    const Animation* m_currentAnim = nullptr; // 再生中アニメ
    float m_timer = 0.0f;                     // フレーム経過時間
    int m_currentFrame = 0;                   // 現在フレーム（相対）
    bool m_isFinished = false;
};