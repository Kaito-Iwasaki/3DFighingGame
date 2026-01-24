//=====================================================================
//
// Grid [Grid.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "Grid.h"
#include "util.h"

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************
#define GRID_SEGMENT_X		(20)
#define GRID_SEGMENT_Z		(20)
#define GRID_SIZE			(50)

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
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffGrid = NULL;

//=====================================================================
// 初期化処理
//=====================================================================
void InitGrid(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	pDevice->CreateVertexBuffer(
		sizeof(VERTEX_3D) * 2 * (GRID_SEGMENT_X + 1) * (GRID_SEGMENT_Z + 1),
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffGrid,
		NULL
	);
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitGrid(void)
{
	RELEASE(g_pVtxBuffGrid);
}

//=====================================================================
// 更新処理
//=====================================================================
void UpdateGrid(void)
{
	VERTEX_3D* pVtx;
	D3DXVECTOR3 posStart = D3DXVECTOR3(-GRID_SEGMENT_X * GRID_SIZE / 2, 0, GRID_SEGMENT_Z * GRID_SIZE / 2);

	g_pVtxBuffGrid->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCountGridX = 0; nCountGridX < GRID_SEGMENT_X + 1; nCountGridX++)
	{
		pVtx[0].pos = D3DXVECTOR3(posStart.x + GRID_SIZE * nCountGridX, 0, posStart.z);
		pVtx[1].pos = D3DXVECTOR3(posStart.x + GRID_SIZE * nCountGridX, 0, -posStart.z);

		pVtx[0].nor = D3DXVECTOR3(0, 1, 0);
		pVtx[1].nor = D3DXVECTOR3(0, 1, 0);

		pVtx[0].col = D3DXCOLOR_WHITE;
		pVtx[1].col = D3DXCOLOR_WHITE;

		pVtx += 2;
	}

	for (int nCountGridZ = 0; nCountGridZ < GRID_SEGMENT_X + 1; nCountGridZ++)
	{
		pVtx[0].pos = D3DXVECTOR3(-posStart.x, 0, posStart.z - GRID_SIZE * nCountGridZ);
		pVtx[1].pos = D3DXVECTOR3(posStart.x, 0, posStart.z - GRID_SIZE * nCountGridZ);

		pVtx[0].nor = D3DXVECTOR3(0, 1, 0);
		pVtx[1].nor = D3DXVECTOR3(0, 1, 0);

		pVtx[0].col = D3DXCOLOR_WHITE;
		pVtx[1].col = D3DXCOLOR_WHITE;

		pVtx += 2;
	}


	g_pVtxBuffGrid->Unlock();
}

//=====================================================================
// 描画処理
//=====================================================================
void DrawGrid(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxWorld;
	D3DXMATRIX mtxRot, mtxTrans;
	VERTEX_3D* pVtx;

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, g_pVtxBuffGrid, 0, sizeof(VERTEX_3D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	for (int nCountLine = 0; nCountLine < (GRID_SEGMENT_X + 1) * (GRID_SEGMENT_Z + 1); nCountLine++)
	{
		// ワールドマトリックスの初期化
		// これでマトリックスを単位行列にする
		D3DXMatrixIdentity(&mtxWorld);

		// 向きを反映
		D3DXMatrixRotationYawPitchRoll(&mtxRot, 0, 0, 0);
		D3DXMatrixMultiply(&mtxWorld, &mtxWorld, &mtxRot);

		// 位置を反映
		D3DXMatrixTranslation(&mtxTrans, 0, 0, 0);
		D3DXMatrixMultiply(&mtxWorld, &mtxWorld, &mtxTrans);

		// ワールドマトリックスの設定
		pDevice->SetTransform(D3DTS_WORLD, &mtxWorld);

		// ポリゴンの描画
		pDevice->DrawPrimitive(D3DPT_LINESTRIP, nCountLine * 2, 1);
	}
}