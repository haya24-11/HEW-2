#include "Animator.h"

/*
    Play
    ----
    ・同じアニメが指定された場合は何もしない
    ・違うアニメなら最初から再生
*/
void Animator::Play(const Animation& anim)
{
    if (m_currentAnim == &anim)
        return;

    m_currentAnim = &anim;
    m_timer = 0.0f;
    m_currentFrame = 0;
    m_isFinished = false;
}

/*
    Update
    ------
    ・deltaTime を使ってフレームを進める
    ・ループ or 停止を制御
*/
void Animator::Update(float deltaTime)
{
    if (!m_currentAnim)
        return;

    m_timer += deltaTime;

    // フレーム更新
    while (m_timer >= m_currentAnim->frameTime)
    {
        m_timer -= m_currentAnim->frameTime;
        m_currentFrame++;

        // 最後まで行ったら
        if (m_currentFrame >= m_currentAnim->frameCount)
        {
            if (m_currentAnim->loop)
            {
                m_currentFrame = 0;
            }
            else
            {
                m_currentFrame = m_currentAnim->frameCount - 1;
                m_isFinished = true;
            }
        }
    }
}

/*
    GetCurrentFrame
    ---------------
    ・スプライトシート上の絶対フレーム番号を返す
*/
int Animator::GetCurrentFrame() const
{
    if (!m_currentAnim)
        return 0;

    return m_currentAnim->startFrame + m_currentFrame;
}
