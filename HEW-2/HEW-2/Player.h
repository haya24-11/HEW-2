#pragma once
#include <vector>
#include "Chara.h"

class Skill;

/*
    Player
    ======
    ・成長要素を持つクラス
    ・Exp
    ・Level
    ・Skill リスト

    ※ 戦闘処理の中身は持たない
*/

class Player : public Chara
{
public:
    Player();

    // Player 固有の更新（今は空）
    void Update(float deltaTime)override;

    // 攻撃の入口
    void Attack() override;

    // スキル取得時の共通処理
    void ApplyAbility(Skill* skill);

private:
    int exp = 0;
    int level = 1;
    std::vector<Skill*> skills; // Skillは所有しない（Mode側管理）
};