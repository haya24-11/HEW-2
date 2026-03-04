#include "Object.h"
#include <unordered_map>
#include <string>
#include "Texture.h"
#include "CameraGlobals.h"

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

	// UVは常にフルテクスチャ
	m_vertexList[0].u = 0.0f;
	m_vertexList[0].v = 0.0f;

	m_vertexList[1].u = 1.0f;
	m_vertexList[1].v = 0.0f;

	m_vertexList[2].u = 0.0f;
	m_vertexList[2].v = 1.0f;

	m_vertexList[3].u = 1.0f;
	m_vertexList[3].v = 1.0f;

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
	SAFE_RELEASE(m_pTextureView);

	m_pTextureView = GetTextureSRV(g_pDevice, imgname);
	if (!m_pTextureView)
	{
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	m_pTextureView->AddRef();

	return S_OK;
}

void Object::Draw()
{
	//if (!m_active) return;
	if (!m_visible) return;

	// =========================
	// 頂点バッファ設定
	// =========================
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	// =========================
	// テクスチャをピクセルシェーダに設定
	// =========================
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
		DirectX::XMMatrixTranslation(
			m_isUI ? m_pos.x : (m_pos.x - g_cameraX),
			m_isUI ? m_pos.y : (m_pos.y - g_cameraY),
			m_pos.z
		);

	DirectX::XMMATRIX matrixWorld =
		matrixScale * matrixAngle * matrixPos;

	// =========================
	// UV変換行列（アニメーション：numU/numV）
	// =========================
	float u = (float)numU * m_frameU;
	float v = (float)numV * m_frameV;

	DirectX::XMMATRIX matrixTex =
		DirectX::XMMatrixScaling(m_frameU, m_frameV, 1.0f) *
		DirectX::XMMatrixTranslation(u, v, 0.0f);

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
	if (m_useAnimFrame)
	{
		Draw(m_animFrame);
		return;
	}

	g_pDeviceContext->Draw(4, 0);
}

/*
	Draw(frameIndex)
	----------------
	frameIndex : 0,1,2,3...
	左上 → 右 → 下 の順で並ぶスプライトを想定
*/
// =====================================================
// スプライトシート描画
// =====================================================
void Object::Draw(int frameIndex)
{
	if (!m_active) return;

	extern float g_cameraX;
	extern float g_cameraY;

	int frameX = frameIndex % m_splitX;
	int frameY = frameIndex / m_splitX;

	float u = frameX * m_frameU;
	float v = frameY * m_frameV;

	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	DirectX::XMMATRIX matrixProj =
		DirectX::XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);

	DirectX::XMMATRIX matrixScale =
		DirectX::XMMatrixScaling(m_size.x, m_size.y, m_size.z);

	DirectX::XMMATRIX matrixAngle =
		DirectX::XMMatrixRotationZ(m_angle * 3.141592f / 180.0f);

	DirectX::XMMATRIX matrixPos =
		DirectX::XMMatrixTranslation(
			m_isUI ? m_pos.x : (m_pos.x - g_cameraX),
			m_isUI ? m_pos.y : (m_pos.y - g_cameraY),
			m_pos.z
		);

	DirectX::XMMATRIX matrixWorld =
		matrixScale * matrixAngle * matrixPos;

	DirectX::XMMATRIX matrixTex;

	if (m_flipX)
	{
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

	g_pDeviceContext->UpdateSubresource(
		g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

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

	m_frameU = 1.0f / splitX;
	m_frameV = 1.0f / splitY;
}

void Object::Uninit()
{
	// =========================
	// 解放処理
	//  - AddRef した分だけ Release する
	// =========================
	SAFE_RELEASE(m_pVertexBuffer);
}

Object* Object::SetPos(float x, float y, float z)
{
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;

	m_collider.SetPosition({ x, y });
	return this;
}


Object* Object::SetSize(float x, float y, float z)
{
	m_size.x = x;
	m_size.y = y;
	m_size.z = z;

	// ✅ 半径（デフォルト）
	const float r = (x + y) * 0.25f;
	m_collider.SetRadius(r);
	collisionRadius = r; // （任意）GetCollisionRadius用に一致させる

	m_collider.SetPosition({ m_pos.x, m_pos.y });
	return this;

}

Object* Object::SetAngle(float a)
{
	// 角度を設定
	m_angle = a;
	return this;

}

Object* Object::SetColor(float r, float g, float b, float a)
{
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
	return this;
}

void Object::SetFlipX(bool flip)
{
	// 左右反転フラグ
	m_flipX = flip;
}

void Object::SetTexture(const char* imgname)
{
	// 既存のテクスチャ（Object が保持していた参照）を解放
	SAFE_RELEASE(m_pTextureView);

	// Texture.cpp 側のキャッシュからテクスチャを取得
	m_pTextureView = GetTextureSRV(g_pDevice, imgname);
	if (!m_pTextureView)
	{
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
		return;
	}

	// ⚠️ 重要：
	// キャッシュは 1 つ参照を保持しているため、
	// Object 側でも安全に使用できるよう参照カウントを 1 増やす
	m_pTextureView->AddRef();
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
	collisionRadius = r;
	m_collider.SetRadius(r); // ✅ これが無いと CheckCollision に効かない
}

float Object::GetCollisionRadius() const
{
	return m_collider.GetRadius(); // ✅ どこから見ても同じ半径になる
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

void Object::SetAnimFrame(int frame)
{
	m_animFrame = frame;
	m_useAnimFrame = true;
}

void Object::SetActive(bool active)
{
	m_active = active;
}

bool Object::IsActive() const
{
	return m_active;
}