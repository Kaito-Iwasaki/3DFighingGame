//=====================================================================
//
// Polygon [Polygon.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "Polygon.h"
#include "util.h"
#include "input.h"
#include "camera.h"


//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************


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
LPDIRECT3DVERTEXBUFFER9 g_pVtxBuffPolygon = NULL;
D3DXVECTOR3 posPolygon;

// ワールドマトリックス
// このオブジェクトの最終的な位置はここに入る
D3DXMATRIX g_mtxWorldPolygon;

//=====================================================================
// 初期化処理
//=====================================================================
void InitPolygon(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	posPolygon = D3DXVECTOR3_ZERO;
	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(
		sizeof(VERTEX_3D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&g_pVtxBuffPolygon,
		NULL
	);
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitPolygon(void)
{
	RELEASE(g_pVtxBuffPolygon);
}

//=====================================================================
// 更新処理
//=====================================================================
void UpdatePolygon(void)
{
	DIMOUSESTATE mouse = GetMouse();
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXVECTOR2 posMouse = GetMousePos();
	D3DXMATRIX mtxView, mtxProj, mtxViewport;
	D3DXMATRIX mtxViewInv, mtxProjInv, mtxViewportInv;
	D3DXVECTOR3 posNear, posFar, posWorld;
	D3DXVECTOR3 vecRay;
	D3DXVECTOR3 vecUp = D3DXVECTOR3(0, 1, 0);

	// スクリーン座標→ワールド
	// = スクリーン座標・逆ビューマトリックス・逆プロジェクションマトリックス・逆ピューポイントマトリックス
	// ビューポイントマトリックスは自分で作る必要アリ
	// [ SCREEN_W / 2, 0, 0, 0 ]
	// [ 0, -SCREEN_H / 2, 0, 0 ]
	// [ 0, 0, 1, 0 ]
	// [ SCREEN_W / 2, SCREEN_H / 2, 0, 1]

	// ビュー行列の取得
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	// プロジェクション行列の取得
	pDevice->GetTransform(D3DTS_PROJECTION, &mtxProj);

	// ビューポート行列の初期化
	D3DXMatrixIdentity(&mtxViewport);
	mtxViewport._11 = SCREEN_WIDTH / 2;
	mtxViewport._22 = -SCREEN_HEIGHT / 2;
	mtxViewport._41 = SCREEN_WIDTH / 2;
	mtxViewport._42 = SCREEN_HEIGHT / 2;

	// 各行列の逆行列を取得
	D3DXMatrixInverse(&mtxViewInv, NULL, &mtxView);
	D3DXMatrixInverse(&mtxProjInv, NULL, &mtxProj);
	D3DXMatrixInverse(&mtxViewportInv, NULL, &mtxViewport);

	D3DXMATRIX mtxInv = mtxViewportInv * mtxProjInv * mtxViewInv;
	D3DXVECTOR3 vecNear = D3DXVECTOR3(posMouse.x, posMouse.y, 0.0f);
	D3DXVECTOR3 vecFar = D3DXVECTOR3(posMouse.x, posMouse.y, 1.0f);
	D3DXVec3TransformCoord(&posNear, &vecNear, &mtxInv);
	D3DXVec3TransformCoord(&posFar, &vecFar, &mtxInv);
	vecRay = Normalize(posFar - posNear);
	D3DXVECTOR3 posNearInv = -posNear;

	if (vecRay.y < 0)
	{
		float Lray = D3DXVec3Dot(&vecRay, &vecUp);
		float LP0 = D3DXVec3Dot(&posNearInv, &vecUp);
		posWorld = posNear + (LP0 / Lray) * vecRay;
	}
	else
	{
		posWorld = posFar;
	}

	if (GetKeyboardPress(DIK_1))
	{
		posWorld.x = 0;
	}
	else if (GetKeyboardPress(DIK_2))
	{
		posWorld.z = 0;
	}

	posPolygon.x = (float)(((int)posWorld.x / 10) * 20);
	posPolygon.z = (float)(((int)posWorld.z / 10) * 20);

	//if (GetKeyboardPress(DIK_LEFT))
	//{
	//	posPolygon.x -= 1;
	//}
	//if (GetKeyboardPress(DIK_RIGHT))
	//{
	//	posPolygon.x += 1;
	//}
	//if (GetKeyboardPress(DIK_UP))
	//{
	//	posPolygon.z += 1;
	//}
	//if (GetKeyboardPress(DIK_DOWN))
	//{
	//	posPolygon.z -= 1;
	//}
}

//=====================================================================
// 描画処理
//=====================================================================
void DrawPolygon(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;

	VERTEX_3D* pVtx;

	// 頂点バッファをロックして頂点情報へのポインタを取得
	g_pVtxBuffPolygon->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点情報を設定
	pVtx[0].pos = D3DXVECTOR3(-10.0f, 0.0f, +10.0f);
	pVtx[1].pos = D3DXVECTOR3(+10.0f, 0.0f, +10.0f);
	pVtx[2].pos = D3DXVECTOR3(-10.0f, 0.0f, -10.0f);
	pVtx[3].pos = D3DXVECTOR3(+10.0f, 0.0f, -10.0f);

	pVtx[0].nor = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	pVtx[1].nor = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	pVtx[2].nor = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	pVtx[3].nor = D3DXVECTOR3(0.0f, -1.0f, 0.0f);

	pVtx[0].col = D3DXCOLOR_WHITE;
	pVtx[1].col = D3DXCOLOR_WHITE;
	pVtx[2].col = D3DXCOLOR_WHITE;
	pVtx[3].col = D3DXCOLOR_WHITE;

	// 頂点バッファをアンロック
	g_pVtxBuffPolygon->Unlock();

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, g_pVtxBuffPolygon, 0, sizeof(VERTEX_3D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	// ワールドマトリックスの初期化
	// これでマトリックスを単位行列にする
	D3DXMatrixIdentity(&g_mtxWorldPolygon);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, 0, 0, 0);
	D3DXMatrixMultiply(&g_mtxWorldPolygon, &g_mtxWorldPolygon, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, posPolygon.x, posPolygon.y, posPolygon.z);
	D3DXMatrixMultiply(&g_mtxWorldPolygon, &g_mtxWorldPolygon, &mtxTrans);

	// ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorldPolygon);

	// ポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
}