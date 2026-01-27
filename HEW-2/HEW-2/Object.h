#pragma once
#include "Renderer.h"
#include"Texture.h"	// テクスチャ読み込み
#include"Collision.h"
class Object {

private:
	// 頂点データ
	Vertex m_vertexList[4] =
	{
		//	x		y		z		r		g		b		a		u		v
		{ -0.5f,  0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	0.0f	},  // 0番目の頂点座標　{ x, y, z } (r,g,b,a}
		{  0.5f,   0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f	},  // 1番目の頂点座標
		{ -0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	1.0f	},  // 2番目の頂点座標
		{  0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f	},  // 3番目の頂点座標

		/*
		//	x		y		z		r		g		b		a		u		v
		{ -0.5f,  0.5f, 0.5f,	1.0f,	0.0f,	1.0f,	1.0f,	0.0f,	0.0f	},  // 0番目の頂点座標　{ x, y, z } (r,g,b,a}
		{  0.5f,   0.5f, 0.5f,	0.0f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f	},  // 1番目の頂点座標
		{ -0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	0.0f,	1.0f,	0.0f,	1.0f	},  // 2番目の頂点座標
		{  0.5f, -0.5f, 0.5f,	1.0f,	0.0f,	1.0f,	1.0f,	1.0f,	1.0f	},  // 3番目の頂点座標
		*/
	};

	// 座標
	DirectX::XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f }; // オブジェクトの座標を代入する変数
	// 大きさ
	DirectX::XMFLOAT3 m_size = { 100.0f,100.0f,0.0f };
	// 角度
	float m_angle = 0.0f;
	// ※オブジェクトの大きさと角度を代入する変数
	// 色
	DirectX::XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f }; // デフォルト色


	// 頂点バッファ
	ID3D11Buffer* m_pVertexBuffer;
	// テクスチャ用変数
	ID3D11ShaderResourceView* m_pTextureView;

	// テクスチャが縦よr子に何分割されているか
	int m_splitX = 1;
	int m_splitY = 1;

// Collision (Circle)
	Collision m_collider;
public:
	// 左上から何段目を切り抜いて表示するか
	float numU = 0;
	float numV = 0;
	HRESULT Init(const char* imgname,int sx=1,int sy=1);	// 初期化
	void Draw();											// 描画
	void Uninit();										// 終了
	void SetPos(float x, float y, float z);	// 座標をセット
	void SetSize(float x, float y, float z); // 大きさをセット
	void SetAngle(float a);						// 角度をセット
	void SetColor(float r, float g, float b, float a); // 色をセット

	// Collision
	// 半径を手動で設定したい場合に使用（未設定なら SetSize() の値から自動計算）
	void SetCollisionRadius(float r);
	// コライダ（円）に直接アクセスしたい場合
	Collision& GetCollider();
	const Collision& GetCollider() const;
	// 他の Object との当たり判定
	bool CheckCollision(const Object& other) const;

	DirectX::XMFLOAT3 GetPos(void);	//	座標をゲット
	DirectX::XMFLOAT3 GetSize(void);	// 大きさをゲット
	float GetAngle(void);						// 角度をセット
	DirectX::XMFLOAT4 GetColor(void);	// 色をセット
};