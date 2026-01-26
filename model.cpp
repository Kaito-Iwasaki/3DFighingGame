//=====================================================================
//
// Model [model.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "model.h"
#include "model_loader.h"
#include "camera.h"

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** 構造体 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** 列挙型 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** プロトタイプ宣言 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** グローバル変数 *****
// 
//*********************************************************************
MODEL g_aModel[MAX_MODEL];
MESHDATA g_aMeshData[MAX_LOADABLE_MODEL];

//=====================================================================
// 初期化処理
//=====================================================================
void InitModel(void)
{
	ZeroMemory(&g_aModel[0], sizeof(MODEL) * MAX_MODEL);
	ZeroMemory(&g_aMeshData[0], sizeof(MESHDATA) * MAX_LOADABLE_MODEL);
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitModel(void)
{
	MESHDATA* pMeshData = &g_aMeshData[0];

	for (int nCntMesh = 0; nCntMesh < MAX_LOADABLE_MODEL; nCntMesh++, pMeshData++)
	{
		ReleaseMesh(pMeshData);
	}
}

//=====================================================================
// 更新処理
//=====================================================================
void UpdateModel(void)
{

}

//=====================================================================
// 描画処理
//=====================================================================
void DrawModel(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	MODEL* pModel = &g_aModel[0];
	D3DXMATRIX mtxRot, mtxTrans;	// 計算用マトリックス
	D3DMATERIAL9 matDef;			// 現在のマテリアル保存用
	D3DXMATERIAL* pMat;				// マテリアルデータへのポインタ

	for (int nCntModel = 0; nCntModel < MAX_MODEL; nCntModel++, pModel++)
	{
		if (pModel->bUsed == false) continue;	// 使用中でないならスキップ

		MESHDATA* pMeshData = &g_aMeshData[pModel->nType];

		// ワールドマトリックスの初期化
		D3DXMatrixIdentity(&pModel->mtxWorld);

		// 向きを反映
		D3DXMatrixRotationYawPitchRoll(
			&mtxRot,
			pModel->transform.rot.y, pModel->transform.rot.x, pModel->transform.rot.z
		);
		D3DXMatrixMultiply(
			&pModel->mtxWorld,
			&pModel->mtxWorld,
			&mtxRot
		);

		// 位置を反映
		D3DXMatrixTranslation(
			&mtxTrans,
			pModel->transform.pos.x, pModel->transform.pos.y, pModel->transform.pos.z
		);
		D3DXMatrixMultiply(
			&pModel->mtxWorld,
			&pModel->mtxWorld,
			&mtxTrans
		);

		// ワールドマトリックスの設定
		pDevice->SetTransform(D3DTS_WORLD, &pModel->mtxWorld);

		// 現在のマテリアルを取得
		pDevice->GetMaterial(&matDef);

		// マテリアルデータへのポインタを取得
		pMat = (D3DXMATERIAL*)pMeshData->pBuffMat->GetBufferPointer();

		for (int nCntMat = 0; nCntMat < (int)pMeshData->dwNumMat; nCntMat++)
		{
			// マテリアルの設定
			pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

			// テクスチャの設定
			pDevice->SetTexture(0, pMeshData->apTexture[nCntMat]);

			// モデル（パーツ）の描画
			pMeshData->pMesh->DrawSubset(nCntMat);
		}

		// 保存していたマテリアルを戻す
		pDevice->SetMaterial(&matDef);
	}
}

//=====================================================================
// モデル設定処理
//=====================================================================
MODEL* SetModel(int nType, D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	MODEL* pModel = &g_aModel[0];

	for (int nCntModel = 0; nCntModel < MAX_MODEL; nCntModel++, pModel++)
	{
		if (pModel->bUsed == true) continue; // 使用中ならスキップ

		// 構造体の初期化
		ZeroMemory(pModel, sizeof(MODEL));
		pModel->bUsed = true;
		pModel->transform.pos = pos;
		pModel->transform.rot = rot;
		pModel->nType = nType;

		return pModel;
	}
}

//=====================================================================
// モデル設定処理（モデル配置情報から）
//=====================================================================
void LoadAndSetModelFromData(MODELDATA* pModelData)
{
	for (int nCountMesh = 0; nCountMesh < pModelData->nNumModel; nCountMesh++)
	{// xファイルの読み込み
		LoadModel(&pModelData->aFilenameModel[nCountMesh][0], nCountMesh);
	}

	for (int nCountModel = 0; nCountModel < pModelData->nCountModelSet; nCountModel++)
	{// モデルの配置
		MODELSETDATA setData = pModelData->aInfoModelSet[nCountModel];

		SetModel(
			setData.nType,
			setData.pos,
			setData.rot
		);
	}
}

//=====================================================================
// モデル読み込み処理
//=====================================================================
void LoadModel(const char* pFilename, int nIdx)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	MESHDATA* pMeshData = &g_aMeshData[nIdx];
	int nNumVtx = 0;
	DWORD dwSizeFVF;
	BYTE* pVtxBuff;

	ReleaseMesh(pMeshData);

	// 頂点の最大・最小位置の初期化
	pMeshData->vtxMax = D3DXVECTOR3(-100000, -100000, -100000);
	pMeshData->vtxMin = D3DXVECTOR3(100000, 100000, 100000);

	// Xファイルの読み込み
	HRESULT hr = D3DXLoadMeshFromX(
		pFilename,
		D3DXMESH_SYSTEMMEM,
		pDevice,
		NULL,
		&pMeshData->pBuffMat,
		NULL,
		&pMeshData->dwNumMat,
		&pMeshData->pMesh
	);

	if FAILED(hr)
	{// モデル読み込みエラー
		char aStrErr[512] = {};
		sprintf(&aStrErr[0], "モデル読み込みエラー\n%s", pFilename);
		MessageBox(GetMainWindow(), &aStrErr[0], "エラー", MB_ICONERROR);
		DestroyWindow(GetMainWindow());
		return;
	}

	// 頂点数を取得
	nNumVtx = pMeshData->pMesh->GetNumVertices();

	// 頂点フォーマットのサイズを取得
	dwSizeFVF = D3DXGetFVFVertexSize(pMeshData->pMesh->GetFVF());

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	pMeshData->pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

	for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++)
	{
		D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;	// 頂点座標を代入

		if (vtx.x < pMeshData->vtxMin.x)
		{// 頂点のX軸最小位置
			pMeshData->vtxMin.x = vtx.x;
		}
		else if (vtx.x > pMeshData->vtxMax.x)
		{// 頂点のX軸最大位置
			pMeshData->vtxMax.x = vtx.x;
		}

		if (vtx.z < pMeshData->vtxMin.z)
		{// 頂点のZ軸最小位置
			pMeshData->vtxMin.z = vtx.z;
		}
		else if (vtx.z > pMeshData->vtxMax.z)
		{// 頂点のZ軸最大位置
			pMeshData->vtxMax.z = vtx.z;
		}

		if (vtx.y < pMeshData->vtxMin.y)
		{// 頂点のY軸最小位置
			pMeshData->vtxMin.y = vtx.y;
		}
		else if (vtx.y > pMeshData->vtxMax.y)
		{// 頂点のY軸最大位置
			pMeshData->vtxMax.y = vtx.y;
		}

		pVtxBuff += dwSizeFVF;
	}

	D3DXMATERIAL* pMat;

	// マテリアルデータへのポインタを取得
	pMat = (D3DXMATERIAL*)pMeshData->pBuffMat->GetBufferPointer();

	// テクスチャの読み込み
	for (int i = 0; i < (int)pMeshData->dwNumMat; i++)
	{
		if (pMat[i].pTextureFilename != NULL)
		{// テクスチャファイルが存在する
			D3DXCreateTextureFromFile(
				pDevice,
				pMat[i].pTextureFilename,
				&pMeshData->apTexture[i]
			);
		}
	}
}

//=====================================================================
// モデル読み込み処理
//=====================================================================
void LoadModel(const char* pFilename, MESHDATA* pMeshData)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	int nNumVtx = 0;
	DWORD dwSizeFVF;
	BYTE* pVtxBuff;

	// メッシュを解放
	ReleaseMesh(pMeshData);

	// 頂点の最大・最小位置の初期化
	pMeshData->vtxMax = D3DXVECTOR3(-100000, -100000, -100000);
	pMeshData->vtxMin = D3DXVECTOR3(100000, 100000, 100000);

	// Xファイルの読み込み
	HRESULT hr = D3DXLoadMeshFromX(
		pFilename,
		D3DXMESH_SYSTEMMEM,
		pDevice,
		NULL,
		&pMeshData->pBuffMat,
		NULL,
		&pMeshData->dwNumMat,
		&pMeshData->pMesh
	);

	if FAILED(hr)
	{// モデル読み込みエラー
		char aStrErr[512] = {};
		sprintf(&aStrErr[0], "モデルを読み込みエラー\n%s", pFilename);
		MessageBox(GetMainWindow(), &aStrErr[0], "エラー", MB_ICONERROR);
		DestroyWindow(GetMainWindow());
		return;
	}

	// 頂点数を取得
	nNumVtx = pMeshData->pMesh->GetNumVertices();

	// 頂点フォーマットのサイズを取得
	dwSizeFVF = D3DXGetFVFVertexSize(pMeshData->pMesh->GetFVF());

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	pMeshData->pMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVtxBuff);

	for (int nCntVtx = 0; nCntVtx < nNumVtx; nCntVtx++)
	{
		D3DXVECTOR3 vtx = *(D3DXVECTOR3*)pVtxBuff;	// 頂点座標を代入

		if (vtx.x < pMeshData->vtxMin.x)
		{// 頂点のX軸最小位置
			pMeshData->vtxMin.x = vtx.x;
		}
		else if (vtx.x > pMeshData->vtxMax.x)
		{// 頂点のX軸最大位置
			pMeshData->vtxMax.x = vtx.x;
		}

		if (vtx.z < pMeshData->vtxMin.z)
		{// 頂点のZ軸最小位置
			pMeshData->vtxMin.z = vtx.z;
		}
		else if (vtx.z > pMeshData->vtxMax.z)
		{// 頂点のZ軸最大位置
			pMeshData->vtxMax.z = vtx.z;
		}

		if (vtx.y < pMeshData->vtxMin.y)
		{// 頂点のY軸最小位置
			pMeshData->vtxMin.y = vtx.y;
		}
		else if (vtx.y > pMeshData->vtxMax.y)
		{// 頂点のY軸最大位置
			pMeshData->vtxMax.y = vtx.y;
		}

		pVtxBuff += dwSizeFVF;
	}

	D3DXMATERIAL* pMat;

	// マテリアルデータへのポインタを取得
	pMat = (D3DXMATERIAL*)pMeshData->pBuffMat->GetBufferPointer();

	// テクスチャの読み込み
	for (int i = 0; i < (int)pMeshData->dwNumMat; i++)
	{
		if (pMat[i].pTextureFilename != NULL)
		{// テクスチャファイルが存在する
			D3DXCreateTextureFromFile(
				pDevice,
				pMat[i].pTextureFilename,
				&pMeshData->apTexture[i]
			);
		}
	}
}

//=====================================================================
// モデル解放処理
//=====================================================================
void ReleaseMesh(MESHDATA* pMeshData)
{
	for (int i = 0; i < MAX_TEXTURE_PER_MODEL; i++)
	{
		if (pMeshData->apTexture[i] != NULL)
		{// テクスチャの破棄
			pMeshData->apTexture[i]->Release();
			pMeshData->apTexture[i] = NULL;
		}
	}

	if (pMeshData->pMesh != NULL)
	{// メッシュの破棄
		pMeshData->pMesh->Release();
		pMeshData->pMesh = NULL;
	}

	if (pMeshData->pBuffMat != NULL)
	{// マテリアルの破棄
		pMeshData->pBuffMat->Release();
		pMeshData->pBuffMat = NULL;
	}
}

//=====================================================================
// モデル衝突判定処理
//=====================================================================
BYTE CollisionModel(D3DXVECTOR3* pos, D3DXVECTOR3 posOld, D3DXVECTOR3 size)
{
	MODEL* pModel = &g_aModel[0];
	BYTE byHitAll = MODEL_HIT_NONE;

	for (int nCntModel = 0; nCntModel < MAX_MODEL; nCntModel++, pModel++)
	{
		if (pModel->bUsed == false) continue;	// 使用中でないならスキップ

		BYTE byHit = MODEL_HIT_NONE;
		D3DXVECTOR3 vtxMin = g_aMeshData[pModel->nType].vtxMin;
		D3DXVECTOR3 vtxMax = g_aMeshData[pModel->nType].vtxMax;

		// モデルの衝突判定処理
		if (
			posOld.x > pModel->transform.pos.x + vtxMin.x
			&& posOld.x < pModel->transform.pos.x + vtxMax.x
			&& posOld.z < pModel->transform.pos.z + vtxMax.z
			&& posOld.z > pModel->transform.pos.z + vtxMin.z
			&& posOld.y < pModel->transform.pos.y + vtxMax.y
			&& posOld.y + size.y > pModel->transform.pos.y + vtxMin.y
			)
		{
			byHit |= MODEL_HIT_IN;
		}

		if (
			posOld.x <= pModel->transform.pos.x + vtxMin.x
			&& pos->x > pModel->transform.pos.x + vtxMin.x
			&& pos->z < pModel->transform.pos.z + vtxMax.z
			&& pos->z > pModel->transform.pos.z + vtxMin.z
			&& pos->y < pModel->transform.pos.y + vtxMax.y
			&& pos->y + size.y >= pModel->transform.pos.y + vtxMin.y
			)
		{// 左
			byHit |= MODEL_HIT_LEFT;
		}
		if (
			posOld.x >= pModel->transform.pos.x + vtxMax.x
			&& pos->x < pModel->transform.pos.x + vtxMax.x
			&& pos->z < pModel->transform.pos.z + vtxMax.z
			&& pos->z > pModel->transform.pos.z + vtxMin.z
			&& pos->y < pModel->transform.pos.y + vtxMax.y
			&& pos->y + size.y >= pModel->transform.pos.y + vtxMin.y
			)
		{// 右
			byHit |= MODEL_HIT_RIGHT;
		}

		if (
			posOld.z <= pModel->transform.pos.z + vtxMin.z
			&& pos->z > pModel->transform.pos.z + vtxMin.z
			&& pos->x > pModel->transform.pos.x + vtxMin.x
			&& pos->x < pModel->transform.pos.x + vtxMax.x
			&& pos->y < pModel->transform.pos.y + vtxMax.y
			&& pos->y + size.y >= pModel->transform.pos.y + vtxMin.y
			)
		{// 前
			byHit |= MODEL_HIT_FRONT;
		}
		if (
			posOld.z >= pModel->transform.pos.z + vtxMax.z
			&& pos->z < pModel->transform.pos.z + vtxMax.z
			&& pos->x > pModel->transform.pos.x + vtxMin.x
			&& pos->x < pModel->transform.pos.x + vtxMax.x
			&& pos->y < pModel->transform.pos.y + vtxMax.y
			&& pos->y + size.y >= pModel->transform.pos.y + vtxMin.y
			)
		{// 後ろ
			byHit |= MODEL_HIT_BACK;
		}

		if (byHit & MODEL_HIT_LEFT)
		{// 左から衝突
			pos->x = pModel->transform.pos.x + vtxMin.x;
			OutputDebugString("Hit Left\n");
		}
		else if (byHit & MODEL_HIT_RIGHT)
		{// 右から衝突
			pos->x = pModel->transform.pos.x + vtxMax.x;
			OutputDebugString("Hit Right\n");
		}
		else if (byHit & MODEL_HIT_FRONT)
		{// 前から衝突
			pos->z = pModel->transform.pos.z + vtxMin.z;
			OutputDebugString("Hit Front\n");
		}
		else if (byHit & MODEL_HIT_BACK)
		{// 後ろから衝突
			pos->z = pModel->transform.pos.z + vtxMax.z;
			OutputDebugString("Hit Back\n");
		}

		if (
			posOld.y >= pModel->transform.pos.y + vtxMax.y
			&& pos->y <= pModel->transform.pos.y + vtxMax.y
			&& pos->x > pModel->transform.pos.x + vtxMin.x
			&& pos->x < pModel->transform.pos.x + vtxMax.x
			&& pos->z > pModel->transform.pos.z + vtxMin.z
			&& pos->z < pModel->transform.pos.z + vtxMax.z
			)
		{// 上
			byHit |= MODEL_HIT_TOP;
		}

		if (
			posOld.y + size.y <= pModel->transform.pos.y + vtxMin.y
			&& pos->y + size.y > pModel->transform.pos.y + vtxMin.y
			&& pos->x > pModel->transform.pos.x + vtxMin.x
			&& pos->x < pModel->transform.pos.x + vtxMax.x
			&& pos->z > pModel->transform.pos.z + vtxMin.z
			&& pos->z < pModel->transform.pos.z + vtxMax.z
			)
		{// 下
			byHit |= MODEL_HIT_BOTTOM;
		}

		if (byHit & MODEL_HIT_TOP)
		{// 上から衝突
			pos->y = pModel->transform.pos.y + vtxMax.y;
		}
		if (byHit & MODEL_HIT_BOTTOM)
		{// 下から衝突
			pos->y = pModel->transform.pos.y + vtxMin.y - size.y;
		}

		byHitAll |= byHit;
	}

	return byHitAll;
}

//=====================================================================
// モデルの上面の位置の取得処理
//=====================================================================
float GetModelGroundHeight(D3DXVECTOR3 pos)
{
	MODEL* pModel = &g_aModel[0];
	float fGround = -1000.0f;

	for (int nCntModel = 0; nCntModel < MAX_MODEL; nCntModel++, pModel++)
	{
		D3DXVECTOR3 vtxMin = g_aMeshData[pModel->nType].vtxMin;
		D3DXVECTOR3 vtxMax = g_aMeshData[pModel->nType].vtxMax;
		float fModelTop = pModel->transform.pos.y + vtxMax.y;

		if (
			pos.x >= pModel->transform.pos.x + vtxMin.x
			&& pos.x <= pModel->transform.pos.x + vtxMax.x
			&& pos.z >= pModel->transform.pos.z + vtxMin.z
			&& pos.z <= pModel->transform.pos.z + vtxMax.z
			&& pos.y >= fModelTop
		)
		{
			if (fModelTop > fGround)
			{
				fGround = fModelTop;
			}
		}
	}

	return fGround;
}