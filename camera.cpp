//=====================================================================
//
// Camera [Camera.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "camera.h"
#include "input.h"
#include "util.h"

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************
#define INIT_CAMERA_ROT				D3DXVECTOR3(D3DXToRadian(-25), 0, 0)		// カメラ回転初期値
#define INIT_CAMERA_OFFSET			D3DXVECTOR3(0, 150, 0)						// カメラオフセット初期値
#define INIT_CAMERA_DISTANCE		(480)										// カメラ距離初期値
#define CAMERA_MIN_DISTANCE			(120)										// カメラ距離最小値
#define CAMERA_MAX_DISTANCE			(2400)										// カメラ距離最大値
#define CAMERA_ROT_X_MIN			(-(D3DX_PI/2) + 0.01f)						// カメラX軸回転最小値
#define CAMERA_ROT_X_MAX			((D3DX_PI/2) - 0.01f)						// カメラX軸回転最大値
#define CAMERA_FREE_MOVESPEED		(5.0f)
#define CAMERA_FREE_DRAGSPEED		(3.0f)
#define CAMERA_FREE_ROTATE_SPEED_V	(0.0075f)
#define CAMERA_FREE_ROTATE_SPEED_R	(0.01f)

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
CAMERA g_camera[CAMERATYPE_MAX];

//=====================================================================
// 初期化処理
//=====================================================================
void InitCamera(void)
{
	ZeroMemory(&g_camera[0], sizeof(g_camera));
	g_camera[CAMERATYPE_GAME].vecU = D3DXVECTOR3_UP;
	g_camera[CAMERATYPE_GAME].rot = INIT_CAMERA_ROT;
	g_camera[CAMERATYPE_GAME].offset = INIT_CAMERA_OFFSET;
	g_camera[CAMERATYPE_GAME].fDistance = INIT_CAMERA_DISTANCE;
	g_camera[CAMERATYPE_GAME].viewport.X = 0;
	g_camera[CAMERATYPE_GAME].viewport.Y = 0;
	g_camera[CAMERATYPE_GAME].viewport.Width = SCREEN_WIDTH ;
	g_camera[CAMERATYPE_GAME].viewport.Height = SCREEN_HEIGHT;
	g_camera[CAMERATYPE_GAME].viewport.MinZ = 0.0f;
	g_camera[CAMERATYPE_GAME].viewport.MaxZ = 1.0f;
	g_camera[CAMERATYPE_GAME].mode = CAMERAMODE_NONE;

	SetCameraPosVFromAngle(0);
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitCamera(void)
{

}

//=====================================================================
// 更新処理
//=====================================================================
void UpdateCamera(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	CAMERA* pCamera = &g_camera[0];

	DIMOUSESTATE mouse = GetMouse();											// マウス
	D3DXVECTOR3 vecForwad = Direction(pCamera->posV, pCamera->posR);			// カメラ正面へのベクトル
	D3DXVECTOR3 vecRight = -Normalize(CrossProduct(vecForwad, D3DXVECTOR3_UP));	// カメラ右方向へのベクトル
	D3DXVECTOR3 vecUp = -Normalize(CrossProduct(vecRight, vecForwad));			// カメラ上方向へのベクトル

	/*カメラ移動処理*/
	if (GetMousePress(MOUSE_LEFT) && GetMousePress(MOUSE_RIGHT))
	{// 平行移動（前後）
		D3DXVECTOR3 move = vecForwad * -mouse.lY * CAMERA_FREE_DRAGSPEED;
		MoveCamera(0, move);
	}
	else if (GetMousePress(MOUSE_MIDDLE))
	{// 平行移動（上下左右）
		D3DXVECTOR3 move = (vecUp * mouse.lY + vecRight * -mouse.lX) * CAMERA_FREE_DRAGSPEED;
		MoveCamera(0, move);
	}
	else if (GetMousePress(MOUSE_LEFT))
	{// 視点操作
		pCamera->rot.x = GetFixedRotation(pCamera->rot.x - mouse.lY * CAMERA_FREE_ROTATE_SPEED_R);
		pCamera->rot.y = GetFixedRotation(pCamera->rot.y + mouse.lX * CAMERA_FREE_ROTATE_SPEED_R);
		SetCameraPosVFromAngle(0);
	}
	else if (GetMousePress(MOUSE_RIGHT))
	{// 注視点操作
		pCamera->rot.x = GetFixedRotation(pCamera->rot.x - mouse.lY * CAMERA_FREE_ROTATE_SPEED_V);
		pCamera->rot.y = GetFixedRotation(pCamera->rot.y + mouse.lX * CAMERA_FREE_ROTATE_SPEED_V);
		SetCameraPosRFromAngle(0);
	}

	if (mouse.lZ != 0)
	{// カメラ距離の設定
		pCamera->fDistance += mouse.lZ;
		Clampf(&pCamera->fDistance, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
		SetCameraPosVFromAngle(0);
	}

	if (GetKeyboardPress(DIK_W))
	{
		MoveCamera(0, vecForwad * 5);
	}
	if (GetKeyboardPress(DIK_D))
	{
		MoveCamera(0, vecRight * 5);
	}
	if (GetKeyboardPress(DIK_E))
	{
		MoveCamera(0, vecUp * 5);
	}

	// ビューマトリックスの初期化
	D3DXMatrixIdentity(&pCamera->mtxView);

	// ビューマトリックスの作成
	D3DXMatrixLookAtLH(
		&pCamera->mtxView,
		&pCamera->posV,
		&pCamera->posR,
		&pCamera->vecU
	);

	// ビューマトリックスの設定
	pDevice->SetTransform(D3DTS_VIEW, &pCamera->mtxView);
}

//=====================================================================
// カメラ設定処理
//=====================================================================
void SetCamera(int nIdx)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	CAMERA* pCamera = &g_camera[nIdx];

	// ビューポートを設定
	pDevice->SetViewport(&pCamera->viewport);

	// プロジェクションマトリックスの初期化
	D3DXMatrixIdentity(&pCamera->mtxProjection);

	// プロジェクションマトリックスを作成（透視投影行列）
	D3DXMatrixPerspectiveFovLH(
		&pCamera->mtxProjection,
		D3DXToRadian(45.0f),							// 視野角
		(float)pCamera->viewport.Width / (float)pCamera->viewport.Height,		// アスペクト比
		10.0f,											// 最小Z値
		5000.0f											// 最大Z値
	);

	// プロジェクションマトリックスの設定
	pDevice->SetTransform(D3DTS_PROJECTION, &pCamera->mtxProjection);

	// ビューマトリックスの初期化
	D3DXMatrixIdentity(&pCamera->mtxView);

	// ビューマトリックスの作成
	D3DXMatrixLookAtLH(
		&pCamera->mtxView,
		&pCamera->posV,
		&pCamera->posR,
		&pCamera->vecU
	);

	// ビューマトリックスの設定
	pDevice->SetTransform(D3DTS_VIEW, &pCamera->mtxView);
}

//=====================================================================
// カメラ取得処理
//=====================================================================
CAMERA* GetCamera(int nIdx)
{
	return &g_camera[nIdx];
}

//=====================================================================
// カメラ移動処理
//=====================================================================
void MoveCamera(int nIdx, D3DXVECTOR3 move)
{
	CAMERA* pCamera = &g_camera[nIdx];

	// カメラの視点と注視点を同量動かす（平行移動）
	pCamera->posV += move;
	pCamera->posR += move;
}

//=====================================================================
// 視点の位置設定処理
//=====================================================================
void SetCameraPosV(int nIdx, D3DXVECTOR3 pos)
{
	CAMERA* pCamera = &g_camera[nIdx];

	pCamera->posV = pos;
}

//=====================================================================
// 注視点の位置設定処理
//=====================================================================
void SetCameraPosR(int nIdx, D3DXVECTOR3 pos)
{
	CAMERA* pCamera = &g_camera[nIdx];

	pCamera->posR = pos;
}

//=====================================================================
// 角度から視点の位置を設定する処理
//=====================================================================
void SetCameraPosVFromAngle(int nIdx)
{
	CAMERA* pCamera = &g_camera[nIdx];

	Clampf(&pCamera->rot.x, -(D3DX_PI / 2) + 0.01f, (D3DX_PI / 2) - 0.01f);

	// 角度から視点の位置を求める
	pCamera->posV.x = pCamera->posR.x - sinf(pCamera->rot.y) * cosf(pCamera->rot.x) * pCamera->fDistance;
	pCamera->posV.z = pCamera->posR.z - cosf(pCamera->rot.y) * cosf(pCamera->rot.x) * pCamera->fDistance;
	pCamera->posV.y = pCamera->posR.y - sinf(pCamera->rot.x) * pCamera->fDistance;
}

//=====================================================================
// 角度から注視点の位置を設定する処理
//=====================================================================
void SetCameraPosRFromAngle(int nIdx)
{
	CAMERA* pCamera = &g_camera[nIdx];

	Clampf(&pCamera->rot.x, -(D3DX_PI/2) + 0.01f, (D3DX_PI / 2) - 0.01f);

	// 角度から注視点の位置を求める
	pCamera->posR.x = pCamera->posV.x + sinf(pCamera->rot.y) * cosf(pCamera->rot.x) * pCamera->fDistance;
	pCamera->posR.z = pCamera->posV.z + cosf(pCamera->rot.y) * cosf(pCamera->rot.x) * pCamera->fDistance;
	pCamera->posR.y = pCamera->posV.y + sinf(pCamera->rot.x) * pCamera->fDistance;
}

//=====================================================================
// カメラ視点回転処理
//=====================================================================
void RotateCameraByV(int nIdx, D3DXVECTOR3 rotMove)
{
	CAMERA* pCamera = &g_camera[nIdx];

	// カメラの角度を変更
	pCamera->rot = GetFixedRotation(pCamera->rot + rotMove);

	// 角度から注視点の位置を設定
	SetCameraPosRFromAngle(nIdx);
}

//=====================================================================
// カメラ注視点回転処理
//=====================================================================
void RotateCameraByR(int nIdx, D3DXVECTOR3 rotMove)
{
	CAMERA* pCamera = &g_camera[nIdx];

	// カメラの角度を変更
	pCamera->rot = GetFixedRotation(pCamera->rot + rotMove);

	// 角度から視点の位置を設定
	SetCameraPosVFromAngle(nIdx);
}