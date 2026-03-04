#pragma once

#include <xaudio2.h>


// サウンドファイル
typedef enum
{
	SOUND_LABEL_BGM_TITLE = 0,		// タイトルシーン BGM（初期）
	SOUND_LABEL_BGM_GAME,			// ゲームプレイ BGM
	SOUND_LABEL_BGM_RESULT,			// リザルトシーン BGM
	SOUND_LABEL_BGM_GAMEOVER,	// ゲームオーバーシーン BGM
	SOUND_LABEL_SE_ATTACK_LIGHT,		// サンプルSE
	SOUND_LABEL_SE_ATTACK_HEAVY,		// サンプルSE

	SOUND_LABEL_MAX,
} SOUND_LABEL;

class Sound {
private:
	// パラメータ構造体
	typedef struct
	{
		LPCSTR filename;	// 音声ファイルまでのパスを設定
		bool bLoop;			// trueでループ。通常BGMはtrue、SEはfalse。
	} PARAM;

	PARAM m_param[SOUND_LABEL_MAX] =
	{
		{"asset/BGM/BGM_Title.wav", true},			// タイトルシーン BGM（ループさせるのでtrue設定）
		{"asset/BGM/BGM_GamePlay.wav", true},	// ゲームプレイ BGM
		{"asset/BGM/BGM_Result.wav", true},			// リザルトシーン BGM
		{"asset/BGM/BGM_Result.wav", true},			// ゲームオーバーシーン BGM
		{"asset/SE/se_attack_light.wav", false},  		// サンプルSE（ループしないのでfalse設定）
		{"asset/SE/se_attack_light.wav", false},		// サンプルSE



	};

	IXAudio2* m_pXAudio2 = NULL;
	IXAudio2MasteringVoice* m_pMasteringVoice = NULL;
	IXAudio2SourceVoice* m_pSourceVoice[SOUND_LABEL_MAX];
	WAVEFORMATEXTENSIBLE m_wfx[SOUND_LABEL_MAX]; // WAVフォーマット
	XAUDIO2_BUFFER m_buffer[SOUND_LABEL_MAX];
	BYTE* m_DataBuffer[SOUND_LABEL_MAX] = {};

	HRESULT FindChunk(HANDLE, DWORD, DWORD&, DWORD&);
	HRESULT ReadChunkData(HANDLE, void*, DWORD, DWORD);

	// 音量管理用の変数を追加
	float m_volumeBGM = 1.0f; // デフォルト音量
	float m_volumeSE = 5.0f;

public:
	Sound();
	static Sound* GetInstance();

	// ゲームループ開始前に呼び出すサウンドの初期化処理
	HRESULT Init(void);

	// ゲームループ終了後に呼び出すサウンドの解放処理
	void Uninit(void);

	// 引数で指定したサウンドを再生する
	void Play(SOUND_LABEL label);

	// 引数で指定したサウンドを停止する
	void Stop(SOUND_LABEL label);

	// 引数で指定したサウンドの再生を再開する
	void Resume(SOUND_LABEL label);

	//==============================
	// 【追加】
	// 指定したサウンドの音量を変更する
	// label : 音の識別子
	// volume: 0.0f（無音）～1.0f（最大）
	//==============================
	void SetVolume(SOUND_LABEL label, float volume);

	// 音量調整を追加
	void SetVolumeBGM(float volume);  // BGM専用
	void SetVolumeSE(float volume);   // SE専用

	

};