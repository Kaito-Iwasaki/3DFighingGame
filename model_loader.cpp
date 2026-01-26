//=====================================================================
//
// model_loader [model_loader.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "model_loader.h"
#include "script.h"
#include <cassert>

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************
#define MAX_READABLE_CHAR	(256)

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
void _Read_SCRIPT(FILE* pFile, MODELDATA* pBuffer);
void _Read_MODELSET(FILE* pFile, MODELSETDATA* pBuffer);
void _Read_FIELDSET(FILE* pFile, FIELDSETDATA* pBuffer);
void _Read_WALLSET(FILE* pFile, WALLSETDATA* pBuffer);

//=====================================================================
// スクリプト読み込み処理
//=====================================================================
void LoadScript(const char* pFileName, MODELDATA* pBuffer)
{
	ZeroMemory(pBuffer, sizeof(MODELDATA));

	FILE* pFile = fopen(pFileName, "r");

	if (pFile != NULL)
	{
		char aStrLine[MAX_READABLE_CHAR] = {};

		while (true)
		{
			// 一行読み込む
			if (ReadWord(pFile, &aStrLine[0]) == EOF)
			{// ファイルの最後まで読み込んだら終了する
				break;
			}

			if (strcmp(&aStrLine[0], "SCRIPT") == 0)
			{
				OutputDebugString("[model_loader.cpp] Script Start\n");
				_Read_SCRIPT(pFile, pBuffer);
			}
			
		}

		OutputDebugString("[model_loader.cpp] Script End\n");
		
	}
	else
	{
		OutputDebugString("[model_loader.cpp] Script Failed\n");
	}
}

//=====================================================================
// [SCRIPT]読み込み処理
//=====================================================================
void _Read_SCRIPT(FILE* pFile, MODELDATA* pBuffer)
{
	char aStrLine[MAX_READABLE_CHAR] = {};

	int nTextureCount = 0;		// テクスチャ読み込み数
	int nModelCount = 0;		// モデル読み込み数

	while (true)
	{
		// 一行読み込む
		if (ReadWord(pFile, &aStrLine[0]) == EOF)
		{// ファイルの最後まで読み込んだら終了する
			break;
		}
		else if (strcmp(&aStrLine[0], "END_SCRIPT") == 0)
		{// スクリプトの終了が宣言されたら終了する
			break;
		}

		if (strcmp(&aStrLine[0], "NUM_TEXTURE") == 0)
		{// テクスチャ数読み込み
			fscanf(pFile, " = %d", pBuffer->nNumTexture);
		}
		else if (strcmp(&aStrLine[0], "TEXTURE_FILENAME") == 0)
		{// テクスチャファイル名読み込み
			if (nTextureCount < MAX_LOADABLE_TEXTURE)
			{// 最大読み込み数まで読み込む
				fscanf(pFile, " = %s", pBuffer->aFilenameTexture[nTextureCount][0]);
				nTextureCount++;
			}
		}
		else if (strcmp(&aStrLine[0], "NUM_MODEL") == 0)
		{// モデル数読み込み
			fscanf(pFile, " = %d", &pBuffer->nNumModel);
		}
		else if (strcmp(&aStrLine[0], "MODEL_FILENAME") == 0)
		{// テクスチャファイル名読み込み
			if (nModelCount < MAX_LOADABLE_MODEL)
			{// 最大読み込み数まで読み込む
				fscanf(pFile, " = %s", &pBuffer->aFilenameModel[nModelCount][0]);
				nModelCount++;
			}
		}
		else if (strcmp(&aStrLine[0], "MODELSET") == 0)
		{// モデルセット情報読み込み
			MODELSETDATA* pData = &pBuffer->aInfoModelSet[pBuffer->nCountModelSet];

			_Read_MODELSET(pFile, pData);

			pBuffer->nCountModelSet++;
		}
	}
}

//=====================================================================
// [MODELSET]読み込み処理
//=====================================================================
void _Read_MODELSET(FILE* pFile, MODELSETDATA* pBuffer)
{
	char aStrLine[MAX_READABLE_CHAR] = {};

	while (true)
	{
		// 一行読み込む
		if (ReadWord(pFile, &aStrLine[0]) == EOF)
		{// ファイルの最後まで読み込んだら終了する
			break;
		}

		if (strcmp(&aStrLine[0], "END_MODELSET") == 0)
		{
			break;
		}
		else if (strcmp(&aStrLine[0], "TYPE") == 0)
		{
			fscanf(pFile, " = %d", &pBuffer->nType);
		}
		else if (strcmp(&aStrLine[0], "POS") == 0)
		{
			fscanf(pFile, " = %f %f %f", &pBuffer->pos.x, &pBuffer->pos.y, &pBuffer->pos.z);
		}
		else if (strcmp(&aStrLine[0], "ROT") == 0)
		{
			fscanf(pFile, " = %f %f %f", &pBuffer->rot.x, &pBuffer->rot.y, &pBuffer->rot.z);
		}
	}
}

//=====================================================================
// [FIELDSET]読み込み処理
//=====================================================================
void _Read_FIELDSET(FILE* pFile, FIELDSETDATA* pBuffer)
{
	char aStrLine[MAX_READABLE_CHAR] = {};
}

//=====================================================================
// [WALLSET]読み込み処理
//=====================================================================
void _Read_WALLSET(FILE* pFile, WALLSETDATA* pBuffer)
{
	char aStrLine[MAX_READABLE_CHAR] = {};
}