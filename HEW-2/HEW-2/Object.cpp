#include "Object.h"
#include <unordered_map>
#include <string>

// =========================
// テクスチャキャッシュ
//  - 同じファイルは1回だけ読み込み、以降は再利用する
//  - 参照カウント(AddRef/Release)で安全に共有する
// =========================
static std::unordered_map<std::string, ID3D11ShaderResourceView*> g_textureCache;

// =========================
// 共有頂点バッファ（Quad用）
//  - 全Objectで同じ4頂点を使うので1回だけ作成し共有する
//  - 各Object側では AddRef/Release で参照カウント管理する
// =========================
ID3D11Buffer* Object::s_pSharedVB = nullptr;
bool Object::s_sharedVBReady = false;

// ------------------------------------------------------------
// 内部ユーティリティ：テクスチャをキャッシュ経由で取得
//  - 戻り値は「Objectが1参照(AddRef済み)持った状態」
// ------------------------------------------------------------
static HRESULT GetTextureCached(const char* imgname, ID3D11ShaderResourceView** outSRV)
{
	if (!outSRV) return E_INVALIDARG;
	*outSRV = nullptr;

	auto it = g_textureCache.find(imgname);
	if (it != g_textureCache.end())
	{
		// 既存を再利用（Objectが使う分の参照を増やす）
		*outSRV = it->second;
		if (*outSRV) (*outSRV)->AddRef();
		return S_OK;
	}

	// 初回ロード
	ID3D11ShaderResourceView* srv = nullptr;
	HRESULT hr = LoadTexture(g_pDevice, imgname, &srv);
	if (FAILED(hr)) return hr;

	// キャッシュ側も1参照を保持（安全な共有のため）
	if (srv) srv->AddRef();
	g_textureCache.emplace(imgname, srv);

	// Object側にも1参照を渡す（呼び出し側がReleaseする）
	*outSRV = srv; // srv は今 2参照（cache 1 + object 1）
	return S_OK;
}

HRESULT Object::Init(const char* imgname, int sx, int sy)
{
	// =========================
	// UV座標（スプライト分割数）を設定
	// =========================
	m_splitX = sx;
	m_splitY = sy;

	m_vertexList[1].u = 1.0f / m_splitX;
	m_vertexList[2].v = 1.0f / m_splitY;
	m_vertexList[3].u = 1.0f / m_splitX;
	m_vertexList[3].v = 1.0f / m_splitY;

	// =========================
	// 頂点バッファ作成（共有）
	//  - 初回だけ CreateBuffer
	//  - 各Objectは参照(AddRef)して使う
	// =========================
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(m_vertexList);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pSysMem = m_vertexList;

	HRESULT hr = S_OK;
	if (!s_sharedVBReady)
	{
		hr = g_pDevice->CreateBuffer(&bufferDesc, &subResourceData, &s_pSharedVB);
		if (FAILED(hr)) return hr;

		s_sharedVBReady = true;
	}

	// 共有VBを使う（Object側も1参照持つ）
	m_pVertexBuffer = s_pSharedVB;
	if (m_pVertexBuffer) m_pVertexBuffer->AddRef();

	// =========================
	// テクスチャ読み込み（キャッシュ）
	//  - Object側は1参照(AddRef済み)を受け取る
	// =========================
	SAFE_RELEASE(m_pTextureView); // 念のため既存を解放
	hr = GetTextureCached(imgname, &m_pTextureView);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
		return hr;
	}

	return S_OK;
}

void Object::Draw()
{
	// =========================
	// 頂点バッファ設定
	// =========================
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	// =========================
	// テクスチャをピクセルシェーダに設定
	// =========================
	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	// =========================
	// プロジェクション行列（2D用：正射影）
	// =========================
	DirectX::XMMATRIX matrixProj =
		DirectX::XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);

	// =========================
	// ワールド行列（拡大 + 回転 + 平行移動）
	// =========================
	DirectX::XMMATRIX matrixScale =
		DirectX::XMMatrixScaling(m_size.x, m_size.y, m_size.z);

	DirectX::XMMATRIX matrixAngle =
		DirectX::XMMatrixRotationZ(m_angle * 3.14f / 180.0f);

	DirectX::XMMATRIX matrixPos =
		DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);

	DirectX::XMMATRIX matrixWorld =
		matrixScale * matrixAngle * matrixPos;

	// =========================
	// UV変換行列（アニメーション：numU/numV）
	// =========================
	float u = (float)numU / m_splitX;
	float v = (float)numV / m_splitY;
	DirectX::XMMATRIX matrixTex = DirectX::XMMatrixTranslation(u, v, 0.0f);

	// =========================
	// 定数バッファ更新
	// =========================
	ConstBuffer cb;
	cb.matrixProj = DirectX::XMMatrixTranspose(matrixProj);
	cb.matrixWorld = DirectX::XMMatrixTranspose(matrixWorld);
	cb.matrixTex = DirectX::XMMatrixTranspose(matrixTex);
	cb.color = m_color;

	g_pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);

	// =========================
	// 描画
	// =========================
	g_pDeviceContext->Draw(4, 0);
}

/*
	Draw(frameIndex)
	----------------
	frameIndex : 0,1,2,3...
	左上 → 右 → 下 の順で並ぶスプライトを想定
*/
void Object::Draw(int frameIndex)
{
	// frameIndex から (x,y) を算出
	int frameX = frameIndex % m_splitX;
	int frameY = frameIndex / m_splitX;

	float u = frameX * m_frameU;
	float v = frameY * m_frameV;

	// 頂点バッファ設定
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	// テクスチャ設定
	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	// 行列計算（2D正射影）
	DirectX::XMMATRIX matrixProj =
		DirectX::XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);

	DirectX::XMMATRIX matrixScale =
		DirectX::XMMatrixScaling(m_size.x, m_size.y, m_size.z);

	DirectX::XMMATRIX matrixAngle =
		DirectX::XMMatrixRotationZ(m_angle * 3.141592f / 180.0f);

	DirectX::XMMATRIX matrixPos =
		DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);

	DirectX::XMMATRIX matrixWorld =
		matrixScale * matrixAngle * matrixPos;

	// =========================
	// UV変換行列（スプライトシート + 反転）
	// =========================
	DirectX::XMMATRIX matrixTex;
	if (m_flipX)
	{
		// 左右反転：Uを反転 + 位置補正
		matrixTex =
			DirectX::XMMatrixScaling(-m_frameU, m_frameV, 1.0f) *
			DirectX::XMMatrixTranslation(u + m_frameU, v, 0.0f);
	}
	else
	{
		matrixTex =
			DirectX::XMMatrixScaling(m_frameU, m_frameV, 1.0f) *
			DirectX::XMMatrixTranslation(u, v, 0.0f);
	}

	ConstBuffer cb;
	cb.matrixProj = DirectX::XMMatrixTranspose(matrixProj);
	cb.matrixWorld = DirectX::XMMatrixTranspose(matrixWorld);
	cb.matrixTex = DirectX::XMMatrixTranspose(matrixTex);
	cb.color = m_color;

	g_pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	g_pDeviceContext->Draw(4, 0);
}

/*
	スプライトシート設定
	--------------------
	splitX : 横フレーム数
	splitY : 縦フレーム数
*/
void Object::SetSpriteSheet(int splitX, int splitY)
{
	m_splitX = splitX;
	m_splitY = splitY;

	// 1フレームのUVサイズ
	m_frameU = 1.0f / m_splitX;
	m_frameV = 1.0f / m_splitY;
}

void Object::Uninit()
{
	// =========================
	// 解放処理
	//  - AddRef した分だけ Release する
	// =========================
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pTextureView);
}

void Object::SetPos(float x, float y, float z)
{
	// 座標を設定
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;

	// コライダー位置も更新
	m_collider.SetPosition({ x, y });
}

void Object::SetSize(float x, float y, float z)
{
	// 大きさを設定
	m_size.x = x;
	m_size.y = y;
	m_size.z = z;

	// 半径設定（例：100x100なら radius=50）
	m_collider.SetRadius((x + y) * 0.25f);

	// 位置も念のため更新
	m_collider.SetPosition({ m_pos.x, m_pos.y });
}

void Object::SetAngle(float a)
{
	// 角度を設定
	m_angle = a;
}

void Object::SetColor(float r, float g, float b, float a)
{
	// カラーを設定
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
}

void Object::SetFlipX(bool flip)
{
	// 左右反転フラグ
	m_flipX = flip;
}

void Object::SetTexture(const char* imgname)
{
	// =========================
	// テクスチャ差し替え（キャッシュ経由）
	//  - 直接 LoadTexture するとカクつきや共有破壊の原因になる
	// =========================
	SAFE_RELEASE(m_pTextureView);

	HRESULT hr = GetTextureCached(imgname, &m_pTextureView);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
	}
}

DirectX::XMFLOAT3 Object::GetPos(void)
{
	return m_pos;
}

DirectX::XMFLOAT3 Object::GetSize(void)
{
	return m_size;
}

float Object::GetAngle(void)
{
	return m_angle;
}

DirectX::XMFLOAT4 Object::GetColor(void)
{
	return m_color;
}

// =========================
// Collision
// =========================
void Object::SetCollisionRadius(float r)
{
	m_collider.SetRadius(r);
}

Collision& Object::GetCollider()
{
	return m_collider;
}

const Collision& Object::GetCollider() const
{
	return m_collider;
}

bool Object::CheckCollision(const Object& other) const
{
	return m_collider.Intersects(other.m_collider);
}

void Object::ReleaseTextureCache()
{
	for (auto& kv : g_textureCache)
	{
		if (kv.second) kv.second->Release();
	}
	g_textureCache.clear();

	if (s_pSharedVB)
	{
		s_pSharedVB->Release();
		s_pSharedVB = nullptr;
		s_sharedVBReady = false;
	}
}
