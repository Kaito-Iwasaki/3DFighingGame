//=====================================================================
//
// Game [Game.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "Game.h"
#include "polygon.h"
#include "camera.h"
#include "Grid.h"
#include "model.h"
#include "input.h"


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
MODEL* g_pModel = NULL;

//=====================================================================
// 初期化処理
//=====================================================================
void InitGame(void)
{
	GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);

	InitCamera();
	InitPolygon();
	InitGrid();
	InitModel();

	LoadModel("data\\MODEL\\player.x", 0);

	g_pModel = SetModel(0, D3DXVECTOR3_ZERO, D3DXVECTOR3_ZERO);
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitGame(void)
{
	UninitCamera();
	UninitPolygon();
	UninitGrid();
	UninitModel();
}

//=====================================================================
// 更新処理
//=====================================================================
void UpdateGame(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	UpdateCamera();
	UpdatePolygon();
	UpdateGrid();
	UpdateModel();

	D3DXVECTOR3 posWorld;
	D3DXMATRIX mtxView, mtxProj, mtxViewport;
	D3DXMATRIX mtxViewInv, mtxProjInv, mtxViewportInv;
	D3DXVECTOR2 posMouse = GetMousePos();
	D3DXVECTOR3 posNear = D3DXVECTOR3(posMouse.x, posMouse.y, 0.0f);
	D3DXVECTOR3 posFar = D3DXVECTOR3(posMouse.x, posMouse.y, 1.0f);

	// 流れとしては
	// 1.スクリーン座標からワールド座標の最近点と最遠点を取り始点と方向を取る。
	// 2.

	pDevice->GetTransform(D3DTS_VIEW, &mtxView);
	pDevice->GetTransform(D3DTS_PROJECTION, &mtxProj);
	D3DXMatrixIdentity(&mtxViewport);
	mtxViewport._11 = SCREEN_WIDTH / 2;
	mtxViewport._22 = -SCREEN_HEIGHT / 2;
	mtxViewport._41 = SCREEN_WIDTH / 2;
	mtxViewport._42 = SCREEN_HEIGHT / 2;
	
	D3DXMatrixInverse(&mtxViewInv, NULL, &mtxView);
	D3DXMatrixInverse(&mtxProjInv, NULL, &mtxProj);
	D3DXMatrixInverse(&mtxViewportInv, NULL, &mtxViewport);
	
	D3DXMATRIX mtxInv = mtxViewportInv * mtxProjInv * mtxViewInv;

	D3DXVec3TransformCoord(&posNear, &posNear, &mtxInv);
	D3DXVec3TransformCoord(&posFar, &posFar, &mtxInv);

	D3DXVECTOR3 vecRay = Normalize(posFar - posNear);
	D3DXVECTOR3 vecFieldNor = D3DXVECTOR3(0, 1, 0);
	float fLengthProjRay = DotProduct(vecRay, vecFieldNor);
	float fLengthProjToOrgin = DotProduct(posNear, vecFieldNor);

	if (vecRay.y < 0)
	{
		posWorld = posNear - (fLengthProjToOrgin / fLengthProjRay) * vecRay;
	}
	else
	{
		posWorld = posFar;
	}

	if (GetKeyboardRepeat(DIK_Q, 1))
	{
		g_pModel->transform.rot.y = GetFixedRotation(g_pModel->transform.rot.y + D3DX_PI * 0.01f);
	}
	if (GetKeyboardRepeat(DIK_E, 1))
	{
		g_pModel->transform.rot.y = GetFixedRotation(g_pModel->transform.rot.y - D3DX_PI * 0.01f);
	}

	//g_pModel->transform.pos = posWorld;

	if (GetMouseTrigger(MOUSE_LEFT))
	{
		SetModel(0, posWorld, g_pModel->transform.rot);
	}



}

//=====================================================================
// 描画処理
//=====================================================================
void DrawGame(void)
{
	SetCamera(0);

	DrawPolygon();
	DrawGrid();
	DrawModel();
}