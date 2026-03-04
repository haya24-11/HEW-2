#pragma once
#include <SimpleMath.h>
#include "Effect.h"
#include "AttackDir.h"
#include <unordered_set>

class Scene;
class Object;
class Enemy;
class GamePlay;

class AttackSlashEffect : public Effect
{
public:
    AttackSlashEffect(Scene* scene, Object* owner, bool facingRight, int damage);
    ~AttackSlashEffect();

    void Update(float deltaTime);
    bool IsDead() const { return m_dead; }
    void Uninit();

private:
    Scene* m_scene = nullptr;
    Object* m_object = nullptr;
    Object* m_owner = nullptr;

    // 表示/寿命
    float m_timer = 0.0f;
    float m_lifeTime = 0.18f;     // ✅ 斬撃の表示時間（短めが自然）
    float m_delay = 0.00f;        // ✅ 遅延（必要なら 0.05〜0.1）
    bool  m_dead = false;

    // ダメージ/当たり判定
    int   m_damage = 0;           // ✅ プレイヤー攻撃力
    float m_hitRadius = 45.0f;    // ✅ 判定の広さ（調整ポイント）
    float m_hitActiveTime = 0.12f;// ✅ 判定が有効な時間（調整ポイント）

    bool m_dead = false;

    AttackDir m_dir;

    GamePlay* m_gameplay = nullptr;
    std::unordered_set<Enemy*> m_hitOnce; // ✅ 同じ敵への多重ヒット防止
};