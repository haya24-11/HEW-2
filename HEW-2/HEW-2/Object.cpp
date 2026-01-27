#include "Object.h"

HRESULT Object::Init(const char* imgname,int sx,int sy)
{
	// UV座標を設定
	m_splitX = sx;
	m_splitY = sy;
	m_vertexList[1].u = 1.0f / m_splitX;
	m_vertexList[2].v = 1.0f / m_splitY;
	m_vertexList[3].u = 1.0f / m_splitX;
	m_vertexList[3].v = 1.0 / m_splitY;
	// 頂点バッファを作成する
	// ※頂点バッファ→VRAMに頂点データを置くための機能
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(m_vertexList);// 確保するバッファサイズを指定
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// 頂点バッファ作成を指定
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = m_vertexList;// VRAMに送るデータを指定
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;

	HRESULT hr = g_pDevice->CreateBuffer(&bufferDesc, &subResourceData, &m_pVertexBuffer);
	if (FAILED(hr)) return hr;

	// テクスチャ読み込み
	hr = LoadTexture(g_pDevice, imgname, &m_pTextureView);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "テクスチャ読み込み失敗", "エラー", MB_ICONERROR | MB_OK);
		return hr;
	}

	return S_OK;
}

void Object::Draw()
{
	// 頂点バッファを設定
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	// テクスチャをピクセルシェーダーに渡す
	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	// プロジェクション変換行列を作成
	DirectX::XMMATRIX matrixProj = DirectX::XMMatrixOrthographicLH(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 3.0f);

	// ワールド変換行列を作成
	DirectX::XMMATRIX matrixScale = DirectX::XMMatrixScaling(m_size.x, m_size.y, m_size.z);	// 大きさ
	DirectX::XMMATRIX matrixAngle = DirectX::XMMatrixRotationZ(m_angle * 3.14f / 180);	// 向き
	DirectX::XMMATRIX matrixposition = DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);	// 位置
	DirectX::XMMATRIX matrixWorld = matrixScale * matrixAngle * matrixposition;	// ワールド変換行列を作成

	// UVアニメーションの行列作成
	float u = (float)numU / m_splitX;
	float v = (float)numV / m_splitY;
	DirectX::XMMATRIX matrixTex = DirectX::XMMatrixTranslation(u, v, 0.0f);

	// 定数バッファを更新
	ConstBuffer cb;
	cb.matrixProj = DirectX::XMMatrixTranspose(matrixProj);	// プロジェクション変換行列
	cb.matrixWorld = DirectX::XMMatrixTranspose(matrixWorld);	// ワールド変換行列

	cb.matrixTex = DirectX::XMMatrixTranspose(matrixTex);	// UVアニメーション行列

	// 頂点カラーのデータを作成
	cb.color = m_color;

	// 行列をシェーダーに渡す
	g_pDeviceContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb, 0, 0);
	// ※座標データを「GPUが扱いやすいデータ型」に変換して定数バッファに送る　（「行列（Matrix）というデータ型」）

	g_pDeviceContext->Draw(4, 0); // 描画命令	(頂点の数,0)
}

/*
	Draw(frameIndex)
	----------------
	frameIndex : 0,1,2,3...
	左上 → 右 → 下 の順
*/
void Object::Draw(int frameIndex)
{
	/*
		frameIndex から
		・横位置
		・縦位置
		を求める
	*/
	int frameX = frameIndex % m_splitX;
	int frameY = frameIndex / m_splitX;

	float u = frameX * m_frameU;
	float v = frameY * m_frameV;

	// 頂点バッファ設定
	UINT strides = sizeof(Vertex);
	UINT offsets = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &strides, &offsets);

	g_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureView);

	// 行列計算（既存と同じ）
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

	// ★ UV変換行列（スプライトシート）
	float flipScaleX = m_flipX ? -m_frameU : m_frameU;

	// 左右反転時は u を 1フレーム分ずらす
	float offsetU = m_flipX ? (u + m_frameU) : u;

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

	g_pDeviceContext->UpdateSubresource(
		g_pConstantBuffer, 0, nullptr, &cb, 0, 0
	);

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

	// 1フレーム分のUVサイズ
	m_frameU = 1.0f / m_splitX;
	m_frameV = 1.0f / m_splitY;
}

void Object::Uninit() {
	// 開放処理
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pTextureView);
}

void Object::SetPos(float x, float y, float z) {
	// 座標をセットする
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;
	// ※positionデータを代入する関数
}

void Object::SetSize(float x, float y, float z)
{
	// 大きさをセットする
	m_size.x = x;
	m_size.y = y;
	m_size.z = z;
	// ※sizeデータを代入する関数
}

void Object::SetAngle(float a)
{
	// 角度をセットする
	m_angle = a;
	// ※angleデータを代入する関数
}

void Object::SetColor(float r, float g, float b, float a)
{
	// 色をセットする
	m_color.x = r;
	m_color.y = g;
	m_color.z = b;
	m_color.w = a;
}

void Object::SetFlipX(bool flip)
{
	m_flipX = flip;
}

void Object::SetTexture(const char* imgname)
{
	SAFE_RELEASE(m_pTextureView);
	LoadTexture(g_pDevice, imgname, &m_pTextureView);
}

DirectX::XMFLOAT3 Object::GetPos(void) {
	return m_pos;	// 座標をゲット
}
DirectX::XMFLOAT3 Object::GetSize(void) {
	return m_size;	// 大きさをゲット
}
float Object::GetAngle(void) {
	return m_angle;	//	角度をセット
}
DirectX::XMFLOAT4 Object::GetColor(void) {
	return m_color;	// 色をセット
}