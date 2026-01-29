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
TRANSFORM transformPolygon;

// ワールドマトリックス
// このオブジェクトの最終的な位置はここに入る
D3DXMATRIX g_mtxWorldPolygon;

//=====================================================================
// 初期化処理
//=====================================================================
void InitPolygon(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	transformPolygon.pos = D3DXVECTOR3_ZERO;
	transformPolygon.size = D3DXVECTOR3(100, 0, 100);
	transformPolygon.rot = D3DXVECTOR3_ZERO;

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
	D3DXVECTOR3 posWorld;
	D3DXVECTOR3 vecRay;	// 最近点から最遠点へのベクトル
	D3DXVECTOR3 vecNor = D3DXVECTOR3(0, 1, 0);	// 平面の法線

	// スクリーン→ワールド変換の流れ
	// まずワールド座標からスクリーン座標への変換の順序を逆にする
	// スクリーン座標＝ワールド座標×ビュー行列×プロジェクション行列×ビューポート行列
	// ワールド座標は逆行列を使うと求められる
	// ワールド座標＝スクリーン座標×逆ビューポート行列×逆プロジェクション行列×逆ビュー行列
	// ここで気を付けるのは行列を掛け合わせる順番で、ワールド→スクリーンはそうだし
	// スクリーン→ワールドもビューポート行列から逆に掛け合わせていく必要アリ（行列は順番変えると結果アホほど変わる）
	// また、スクリーン座標からはビュー空間におけるX成分とY成分しか求まらず、Z成分は適宜設定しなければならない。

	// ビューポートマトリックスは自分で作る必要アリ
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
	D3DXVECTOR3 posNear = D3DXVECTOR3(posMouse.x, posMouse.y, 0.0f);
	D3DXVECTOR3 posFar = D3DXVECTOR3(posMouse.x, posMouse.y, 1.0f);
	D3DXVec3TransformCoord(&posNear, &posNear, &mtxInv);
	D3DXVec3TransformCoord(&posFar, &posFar, &mtxInv);

	// やってることとしてはベクトルRayをどれだけ伸ばすか、ということ。
	vecRay = Normalize(posFar - posNear);

	if (vecRay.y < 0)
	{
		// 光線ベクトルの射影長を求める
		// （ここでいう射影長は光線ベクトルの射影ベクトルの長さ）
		// （射影ベクトルはあるベクトルの影のようなベクトル。詳しくは画像でググれ）
		float fLengthProjRay = D3DXVec3Dot(&vecRay, &vecNor);

		// 最近点からから平面までの長さを求める
		// （ここも最近点から平面の一点P0までのベクトルNearToP0の射影長である）
		// （ちなみにここでposNearを使う理由は、P0=原点とした時、posNearは原点からカメラまでのベクトルとなるから）
		float fLengthProjP0 = D3DXVec3Dot(&posNear, &vecNor);

		// 内積さんまじパネぇっすわ。
		// 使いどころが思い浮かぶようで全く思い浮かばない
		// いや便利なんだろうけど。cos取れるの便利なんだろけど。
		posWorld = posNear - (fLengthProjP0 / fLengthProjRay) * vecRay;
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

	//transformPolygon.pos.x = (float)(((int)posWorld.x / (int)(transformPolygon.size.x/2)) * (int)(transformPolygon.size.x/2));
	//transformPolygon.pos.z = (float)(((int)posWorld.z / (int)(transformPolygon.size.z/2)) * (int)(transformPolygon.size.z/2));
	transformPolygon.pos.x = posWorld.x;
	//transformPolygon.pos.y = posWorld.y;
	transformPolygon.pos.z = posWorld.z;
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
	pVtx[0].pos = D3DXVECTOR3(-transformPolygon.size.x / 2.0f, 0.0f, +transformPolygon.size.z / 2.0f);
	pVtx[1].pos = D3DXVECTOR3(+transformPolygon.size.x / 2.0f, 0.0f, +transformPolygon.size.z / 2.0f);
	pVtx[2].pos = D3DXVECTOR3(-transformPolygon.size.x / 2.0f, 0.0f, -transformPolygon.size.z / 2.0f);
	pVtx[3].pos = D3DXVECTOR3(+transformPolygon.size.x / 2.0f, 0.0f, -transformPolygon.size.z / 2.0f);

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
	D3DXMatrixRotationYawPitchRoll(&mtxRot, transformPolygon.rot.x, transformPolygon.rot.y, transformPolygon.rot.z);
	D3DXMatrixMultiply(&g_mtxWorldPolygon, &g_mtxWorldPolygon, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, transformPolygon.pos.x, transformPolygon.pos.y, transformPolygon.pos.z);
	D3DXMatrixMultiply(&g_mtxWorldPolygon, &g_mtxWorldPolygon, &mtxTrans);

	// ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorldPolygon);

	// ポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}