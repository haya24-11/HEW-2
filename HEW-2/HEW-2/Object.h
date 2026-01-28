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