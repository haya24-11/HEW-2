#pragma once

#include <d3d11.h>  // DirectX11を使うためのヘッダーファイル
#include <DirectXMath.h>	// DirextXの数学関連のヘッダーファイル

#define SCREEN_WIDTH (640)	// ウインドウの幅
#define SCREEN_HEIGHT (480)	// ウインドウの高さ

// 関数のプロトタイプ宣言
HRESULT RendererInit(HWND hwnd);
void    RendererDrawStart();
void    RendererDrawEnd();
void    RendererUninit();

// 構造体の定義
// 頂点データを表す構造体
struct Vertex
{
	// 頂点の位置座標
	float x, y, z;

	//	頂点の色
	float r, g, b, a;

	// テクスチャ座標（uv座標)
	float u, v;
};

// 定数バッファ用構造体
struct ConstBuffer
{
	// 頂点カラー行列
	DirectX::XMFLOAT4 color;
	// UV座標行列
	DirectX::XMMATRIX matrixTex;	// UV座標を変更するデータを定数バッファに送る
	// 投影行列
	DirectX::XMMATRIX matrixProj; // 映り方のデータ
	// ワールド変換行列
	DirectX::XMMATRIX matrixWorld; // 位置・大きさ・向きのデータ
	
};

extern ID3D11Device* g_pDevice;	// デバイス=DirectXの各種機能を作る
// コンテキスト＝描画関連を司る機能
extern ID3D11DeviceContext* g_pDeviceContext;
extern ID3D11Buffer* g_pConstantBuffer;

// Direct3D解放の簡略化マクロ
#define SAFE_RELEASE(p) { if( NULL != p ) { p->Release(); p = NULL; } }