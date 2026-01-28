#pragma once
#include <string>
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "Renderer.h"
#include "Texture.h"     // テクスチャ読み込み
#include "Collision.h"

// ============================================================
// Object
// ------------------------------------------------------------
// ・2Dスプライト(Quad)描画用の基本クラス
// ・位置 / サイズ / 角度 / 色
// ・スプライトシート(分割)描画
// ・当たり判定（円）
// ・頂点バッファ/テクスチャを保持
//
// ※最適化対応：
//  - 共有頂点バッファ（Quad用）
//  - テクスチャキャッシュは Object.cpp 側で管理
// ============================================================
class Object
{
private:
    // =========================================================
    // 頂点データ（Quad: 4頂点）
    //  - 初期値は中心(0,0)基準の四角形
    // =========================================================
    Vertex m_vertexList[4] =
    {
        //    x      y     z     r     g     b     a     u     v
        { -0.5f,  0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f }, // 0: 左上
        {  0.5f,  0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f }, // 1: 右上
        { -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f }, // 2: 左下
        {  0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f }, // 3: 右下
    };

    // =========================================================
    // Transform / Color
    // =========================================================
    DirectX::XMFLOAT3 m_pos = { 0.0f, 0.0f, 0.0f };          // 位置
    DirectX::XMFLOAT3 m_size = { 100.0f, 100.0f, 0.0f };      // サイズ
    float             m_angle = 0.0f;                         // 角度（Z回転）
    DirectX::XMFLOAT4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };    // 頂点カラー

    // =========================================================
    // GPU Resource
    // =========================================================
    ID3D11Buffer* m_pVertexBuffer = nullptr;    // 頂点バッファ（共有VBの参照を持つ）
    ID3D11ShaderResourceView* m_pTextureView = nullptr;    // テクスチャ（SRV）

    // ⚠ 未使用の可能性あり（現状 m_pTextureView を使っているなら不要）
    ID3D11ShaderResourceView* m_texture = nullptr;

    // ⚠ 未使用の可能性あり（m_pos / m_size と二重管理になる）
    DirectX::SimpleMath::Vector3 position{};
    DirectX::SimpleMath::Vector3 size{};

    // =========================================================
    // SpriteSheet（分割数）
    // =========================================================
    int m_splitX = 1;   // 横分割数
    int m_splitY = 1;   // 縦分割数
    float m_frameU = 1.0f; // 1フレーム分のUサイズ
    float m_frameV = 1.0f; // 1フレーム分のVサイズ

    // =========================================================
    // Collision（Circle）
    // =========================================================
    Collision m_collider;

    // =========================================================
    // 表示反転
    // =========================================================
    bool m_flipX = false; // 左右反転フラグ

    // =========================================================
    // 共有頂点バッファ（Quad用）
    //  - Object.cpp 側で初回だけ CreateBuffer
    // =========================================================
    static ID3D11Buffer* s_pSharedVB;
    static bool s_sharedVBReady;

public:
    // =========================================================
    // UVアニメーション用（旧方式：numU/numV を直接使う場合）
    // =========================================================
    float numU = 0.0f;
    float numV = 0.0f;

public:
    // =========================================================
    // 基本
    // =========================================================
    // imgname : テクスチャファイル
    // sx, sy  : スプライトシート分割（省略時 1,1）
    HRESULT Init(const char* imgname, int sx = 1, int sy = 1);

    // 描画（通常）
    void Draw();

    // 描画（スプライトシート：frameIndex 指定）
    void Draw(int frameIndex);

    // スプライトシート設定（分割数を設定）
    void SetSpriteSheet(int splitX, int splitY);

    // 終了（保持リソース解放）
    void Uninit();

    // =========================================================
    // Transform Setter
    // =========================================================
    void SetPos(float x, float y, float z);
    void SetSize(float x, float y, float z);
    void SetAngle(float a);
    void SetColor(float r, float g, float b, float a);

    // =========================================================
    // Collision
    //  - SetSize で半径を計算する方式とは別に、手動で半径設定したい場合に使用
    // =========================================================
    void SetCollisionRadius(float r);
    Collision& GetCollider();
    const Collision& GetCollider() const;

    // 他のObjectとの当たり判定
    bool CheckCollision(const Object& other) const;

    // =========================================================
    // 表示反転 / テクスチャ差し替え
    // =========================================================
    void SetFlipX(bool flip);
    void SetTexture(const char* imgname);

    // =========================================================
    // Getter
    // =========================================================
    DirectX::XMFLOAT3 GetPos(void);
    DirectX::XMFLOAT3 GetSize(void);
    float GetAngle(void);
    DirectX::XMFLOAT4 GetColor(void);

    // =========================================================
    // 共有キャッシュ解放（アプリ終了時に1回だけ呼ぶ）
    //  - テクスチャキャッシュ（SRV）
    //  - 共有頂点バッファ
    // =========================================================
    static void ReleaseTextureCache();

    // カメラの表示範囲（プロジェクションに使う）
    static void SetViewSize(float w, float h);
    // Collision 半径取得（押し出し計算用）
    float GetCollisionRadius() const { return m_collider.GetRadius(); }
};
