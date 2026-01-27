#pragma once
#include <string>
#include <DirectXMath.h>
#include "Renderer.h"
#include"Texture.h"	// テクスチャ読み込み
#include <SimpleMath.h>
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

	ID3D11ShaderResourceView* m_texture = nullptr;

	DirectX::SimpleMath::Vector3 position{};
	DirectX::SimpleMath::Vector3 size{};

	// テクスチャが縦よr子に何分割されているか
	int m_splitX = 1;	// 横分割数
	int m_splitY = 1;	// 縦分割数

	float m_frameU = 1.0f; // 1フレームのUサイズ
	float m_frameV = 1.0f; // 1フレームのVサイズ
public:
	float numU = 0;
	float numV = 0;
	HRESULT Init(const char* imgname,int sx=1,int sy=1);	// 初期化
	void Draw();							// 通常描画
	void Draw(int frameIndex);	//  アニメーション描画
	//  スプライトシート情報設定
	void SetSpriteSheet(int splitX, int splitY);

	void Uninit();										// 終了
	void SetPos(float x, float y, float z);	// 座標をセット
	void SetSize(float x, float y, float z); // 大きさをセット
	void SetAngle(float a);						// 角度をセット
	void SetColor(float r, float g, float b, float a); // 色をセット


	DirectX::XMFLOAT3 GetPos(void);	//	座標をゲット
	DirectX::XMFLOAT3 GetSize(void);	// 大きさをゲット
	float GetAngle(void);						// 角度をセット
	DirectX::XMFLOAT4 GetColor(void);	// 色をセット
};