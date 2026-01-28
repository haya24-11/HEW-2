#include "Texture.h"
#include "stb_image.h"
#include <unordered_map>
#include <string>
#include <windows.h>

// ===============================
// 内部テクスチャキャッシュ
//  - 同じファイルパスのテクスチャは1回だけ生成
//  - 以降はキャッシュした SRV を再利用する
// ===============================
static std::unordered_map<std::string, ID3D11ShaderResourceView*> g_texCache;

// ===============================
// 実際のテクスチャロード処理（キャッシュなし）
//  - ファイル読み込み
//  - GPU テクスチャ生成
//  - ShaderResourceView 作成
// ===============================
HRESULT LoadTexture(
    ID3D11Device* device,
    const char* filename,
    ID3D11ShaderResourceView** srv
)
{
    *srv = nullptr;

    int width, height, bpp;
    unsigned char* pixels = stbi_load(filename, &width, &height, &bpp, 4);
    if (!pixels)
    {
        MessageBoxA(nullptr, filename, "load error", MB_OK);
        return S_FALSE;
    }

    // テクスチャ2Dの設定
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA 8bit
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // 初期データ（CPU → GPU）
    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = pixels;
    sub.SysMemPitch = width * 4;

    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &sub, &texture);
    if (FAILED(hr))
    {
        stbi_image_free(pixels);
        return hr;
    }

    // ShaderResourceView を作成
    hr = device->CreateShaderResourceView(texture, nullptr, srv);

    // SRV が参照を保持するため、Texture2D はここで解放
    texture->Release();

    // CPU 側の画像データを解放
    stbi_image_free(pixels);

    return hr;
}

// ===============================
// キャッシュから SRV を取得
//  - 既に読み込まれていれば再利用
//  - 未ロードの場合のみ LoadTexture を実行
// ===============================
ID3D11ShaderResourceView* GetTextureSRV(
    ID3D11Device* device,
    const char* filename
)
{
    auto it = g_texCache.find(filename);
    if (it != g_texCache.end())
    {
        // 既にキャッシュ済みのテクスチャを返す
        return it->second;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    if (FAILED(LoadTexture(device, filename, &srv)) || !srv)
        return nullptr;

    // キャッシュに登録
    g_texCache[filename] = srv;

    return srv;
}

// ===============================
// テクスチャ事前ロード
//  - ゲーム開始時などに呼び出して
//    実行中のカクつきを防ぐ
// ===============================
void PreloadTexture(
    ID3D11Device* device,
    const char* filename
)
{
    // キャッシュに登録されるだけで、描画はしない
    (void)GetTextureSRV(device, filename);
}
