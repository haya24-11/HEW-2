#pragma once
#include <string>
#include <DirectXMath.h>
#include "Renderer.h"
#include"Texture.h"	// �e�N�X�`���ǂݍ���
#include <SimpleMath.h>
#include"Collision.h"
class Object {

private:
	// ���_�f�[�^
	Vertex m_vertexList[4] =
	{
		//	x		y		z		r		g		b		a		u		v
		{ -0.5f,  0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	0.0f	},  // 0�Ԗڂ̒��_���W�@{ x, y, z } (r,g,b,a}
		{  0.5f,   0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f	},  // 1�Ԗڂ̒��_���W
		{ -0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	1.0f	},  // 2�Ԗڂ̒��_���W
		{  0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f	},  // 3�Ԗڂ̒��_���W

		/*
		//	x		y		z		r		g		b		a		u		v
		{ -0.5f,  0.5f, 0.5f,	1.0f,	0.0f,	1.0f,	1.0f,	0.0f,	0.0f	},  // 0�Ԗڂ̒��_���W�@{ x, y, z } (r,g,b,a}
		{  0.5f,   0.5f, 0.5f,	0.0f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f	},  // 1�Ԗڂ̒��_���W
		{ -0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	0.0f,	1.0f,	0.0f,	1.0f	},  // 2�Ԗڂ̒��_���W
		{  0.5f, -0.5f, 0.5f,	1.0f,	0.0f,	1.0f,	1.0f,	1.0f,	1.0f	},  // 3�Ԗڂ̒��_���W
		*/
	};

	// ���W
	DirectX::XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f }; // �I�u�W�F�N�g�̍��W��������ϐ�
	// �傫��
	DirectX::XMFLOAT3 m_size = { 100.0f,100.0f,0.0f };
	// �p�x
	float m_angle = 0.0f;
	// ���I�u�W�F�N�g�̑傫���Ɗp�x��������ϐ�
	// �F
	DirectX::XMFLOAT4 m_color = { 1.0f,1.0f,1.0f,1.0f }; // �f�t�H���g�F


	// ���_�o�b�t�@
	ID3D11Buffer* m_pVertexBuffer;
	// �e�N�X�`���p�ϐ�
	ID3D11ShaderResourceView* m_pTextureView;

	ID3D11ShaderResourceView* m_texture = nullptr;

	DirectX::SimpleMath::Vector3 position{};
	DirectX::SimpleMath::Vector3 size{};

	// �e�N�X�`�����c��r�q�ɉ���������Ă��邩
	int m_splitX = 1;	// ��������
	int m_splitY = 1;	// �c������

	float m_frameU = 1.0f; // 1�t���[����U�T�C�Y
	float m_frameV = 1.0f; // 1�t���[����V�T�C�Y

// Collision (Circle)
	Collision m_collider;
	bool m_flipX = false;  // �� ���E���]�t���O
public:
	float numU = 0;
	float numV = 0;
	HRESULT Init(const char* imgname,int sx=1,int sy=1);	// ������
	void Draw();							// �ʏ�`��
	void Draw(int frameIndex);	//  �A�j���[�V�����`��
	//  �X�v���C�g�V�[�g���ݒ�
	void SetSpriteSheet(int splitX, int splitY);

	void Uninit();										// �I��
	void SetPos(float x, float y, float z);	// ���W���Z�b�g
	void SetSize(float x, float y, float z); // �傫�����Z�b�g
	void SetAngle(float a);						// �p�x���Z�b�g
	void SetColor(float r, float g, float b, float a); // �F���Z�b�g

	// Collision
	// ���a���蓮�Őݒ肵�����ꍇ�Ɏg�p�i���ݒ�Ȃ� SetSize() �̒l���玩���v�Z�j
	void SetCollisionRadius(float r);
	// �R���C�_�i�~�j�ɒ��ڃA�N�Z�X�������ꍇ
	Collision& GetCollider();
	const Collision& GetCollider() const;
	// ���� Object �Ƃ̓����蔻��
	bool CheckCollision(const Object& other) const;
	void SetFlipX(bool flip);

	void SetTexture(const char* imgname);

	DirectX::XMFLOAT3 GetPos(void);	//	���W���Q�b�g
	DirectX::XMFLOAT3 GetSize(void);	// �傫�����Q�b�g
	float GetAngle(void);						// �p�x���Z�b�g
	DirectX::XMFLOAT4 GetColor(void);	// �F���Z�b�g
};
#pragma once
//#include <string>
//#include <DirectXMath.h>
//#include "Renderer.h"
//#include "Texture.h"   // テクスチャ読み込み
//#include <SimpleMath.h>
//#include "Collision.h"
//
//class Object {
//
//private:
//	// 頂点データ
//	Vertex m_vertexList[4] =
//	{
//		//	x		y		z		r		g		b		a		u		v
//		{ -0.5f,  0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	0.0f },  // 0番目の頂点座標 { x, y, z }
//		{  0.5f,  0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f },  // 1番目の頂点座標
//		{ -0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	0.0f,	1.0f },  // 2番目の頂点座標
//		{  0.5f, -0.5f, 0.5f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f,	1.0f },  // 3番目の頂点座標
//	};
//
//	// 座標
//	DirectX::XMFLOAT3 m_pos = { 0.0f, 0.0f, 0.0f }; // オブジェクトの座標を管理する変数
//
//	// 大きさ
//	DirectX::XMFLOAT3 m_size = { 100.0f, 100.0f, 0.0f };
//
//	// 角度
//	float m_angle = 0.0f;
//
//	// オブジェクトの大きさと角度を管理する変数
//
//	// 色
//	DirectX::XMFLOAT4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルト色
//
//	// 頂点バッファ
//	ID3D11Buffer* m_pVertexBuffer;
//
//	// テクスチャ用変数
//	ID3D11ShaderResourceView* m_pTextureView;
//	ID3D11ShaderResourceView* m_texture = nullptr;
//
//	DirectX::SimpleMath::Vector3 position{};
//	DirectX::SimpleMath::Vector3 size{};
//
//	// テクスチャが縦横に分割されているか
//	int m_splitX = 1;	// 横分割数
//	int m_splitY = 1;	// 縦分割数
//
//	float m_frameU = 1.0f; // 1フレームのUサイズ
//	float m_frameV = 1.0f; // 1フレームのVサイズ
//
//	// Collision（円）
//	Collision m_collider;
//	bool m_flipX = false;  // 左右反転フラグ
//
//public:
//	float numU = 0;
//	float numV = 0;
//
//	HRESULT Init(const char* imgname, int sx = 1, int sy = 1); // 初期化
//	void Draw();                          // 通常描画
//	void Draw(int frameIndex);            // アニメーション描画
//
//	// スプライトシート設定
//	void SetSpriteSheet(int splitX, int splitY);
//
//	void Uninit();                        // 終了処理
//	void SetPos(float x, float y, float z);   // 座標設定
//	void SetSize(float x, float y, float z);  // サイズ設定
//	void SetAngle(float a);                   // 角度設定
//	void SetColor(float r, float g, float b, float a); // 色設定
//
//	// Collision
//	// 半径を直接指定したい場合に使用（未指定なら SetSize() の値から自動計算）
//	void SetCollisionRadius(float r);
//
//	// コライダー（円）に直接アクセスしたい場合
//	Collision& GetCollider();
//	const Collision& GetCollider() const;
//
//	// 他の Object との当たり判定
//	bool CheckCollision(const Object& other) const;
//
//	void SetFlipX(bool flip);
//
//	void SetTexture(const char* imgname);
//
//	DirectX::XMFLOAT3 GetPos(void);    // 座標取得
//	DirectX::XMFLOAT3 GetSize(void);   // サイズ取得
//	float GetAngle(void);              // 角度取得
//	DirectX::XMFLOAT4 GetColor(void);  // 色取得
//};
