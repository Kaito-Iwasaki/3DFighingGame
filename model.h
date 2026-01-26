//=====================================================================
//
// model.cppのヘッダファイル [model.h]
// Author : 
// 
//=====================================================================
#ifndef _MODEL_H_
#define _MODEL_H_

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "main.h"
#include "util.h"
#include "model_loader.h"

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************
#define MAX_TEXTURE_PER_MODEL		(32)

#define MODEL_HIT_NONE				(0x00)
#define MODEL_HIT_IN				(0x01)
#define MODEL_HIT_FRONT				(0x02)
#define MODEL_HIT_BACK				(0x04)
#define MODEL_HIT_LEFT				(0x08)
#define MODEL_HIT_RIGHT				(0x10)
#define MODEL_HIT_TOP				(0x20)
#define MODEL_HIT_BOTTOM			(0x40)

//*********************************************************************
// 
// ***** 列挙型 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** 構造体 *****
// 
//*********************************************************************
// 
typedef struct
{
	TRANSFORM transform;			// 位置・回転・サイズ
	D3DXMATRIX mtxWorld;			// ワールドマトリックス
	int nType;						// モデル番号
	bool bUsed;						// 使用中か
	bool bCollision;				// 衝突判定
}MODEL;

typedef struct
{
	LPDIRECT3DTEXTURE9 apTexture[MAX_TEXTURE_PER_MODEL];	// テクスチャへのポインタ
	LPD3DXMESH pMesh;										// メッシュ情報へのポインタ
	LPD3DXBUFFER pBuffMat;									// マテリアルへのポインタ
	DWORD dwNumMat;											// マテリアル数
	D3DXVECTOR3 vtxMin, vtxMax;								// 端の頂点位置
}MESHDATA;

//*********************************************************************
// 
// ***** プロトタイプ宣言 *****
// 
//*********************************************************************
void InitModel(void);
void UninitModel(void);
void UpdateModel(void);
void DrawModel(void);
MODEL* SetModel(int nType, D3DXVECTOR3 pos, D3DXVECTOR3 rot);
void LoadAndSetModelFromData(MODELDATA* pModelData);
void LoadModel(const char* pFilename, int nIdx);
void LoadModel(const char* pFilename, MESHDATA* pMeshData);
void ReleaseMesh(MESHDATA* pMeshData);
BYTE CollisionModel(D3DXVECTOR3* pos, D3DXVECTOR3 posOld, D3DXVECTOR3 size = D3DXVECTOR3_ZERO);
float GetModelGroundHeight(D3DXVECTOR3 pos);

#endif