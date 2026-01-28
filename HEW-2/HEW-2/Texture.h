#pragma once
#include <d3d11.h>

// 既存
//  - テクスチャファイルを読み込み、
//    ID3D11ShaderResourceView を生成する低レベル関数
HRESULT LoadTexture(
    ID3D11Device* device,
    const char* filename,
    ID3D11ShaderResourceView** srv
);

// ⭐ 追加（キャッシュ用）
//  - 同じファイルは一度だけロードし、
//    以降はキャッシュされた SRV を返す
ID3D11ShaderResourceView* GetTextureSRV(
    ID3D11Device* device,
    const char* filename
);

// （任意）テクスチャ事前ロード
//  - ゲーム開始時などに呼び出し、
//    実行中のカクつきを防止する
void PreloadTexture(
    ID3D11Device* device,
    const char* filename
);
