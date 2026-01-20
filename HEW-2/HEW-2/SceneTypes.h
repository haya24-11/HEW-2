#pragma once

enum class SceneType
{
    Title,
    Play,
    Result,
    //ƒV[ƒ“‚ª‘‚¦‚é‚È‚ç‚±‚±ˆÈã‚Å’Ç‹L
    GameOver,
    NONE = -1
};

enum class Status
{
    Normal,
    Stunned,
    Dead,
    NONE = -1
};