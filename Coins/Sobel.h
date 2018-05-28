#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

void RmwSetImageBoundary(BYTE *BinImg, int width, int height, BYTE Color)
{
	BYTE *pCur1, *pCur2;
	int x, y;

	for (x = 0, pCur1 = BinImg, pCur2 = BinImg + (height - 1)*width; x<width; x++)
		*(pCur1++) = *(pCur2++) = Color;
	for (y = 0, pCur1 = BinImg, pCur2 = BinImg + width - 1; y<height; y++, pCur1 += width, pCur2 += width)
		*pCur1 = *pCur2 = Color;
}

void RmwSobelGrayImage(BYTE *pGryImg, int width, int height, BYTE *pSbImg)
{
	int i, j, G1, G2, x2, y2;
	BYTE *pGry, *pSb, *pCurGry, *pCurSb;

	x2 = width - 1;
	y2 = height - 1;
	RmwSetImageBoundary(pSbImg, width, height, 0);
	for (i = 1, pGry = pGryImg + width, pSb = pSbImg + width; i<y2; i++, pGry += width, pSb += width)
	{
		for (j = 1, pCurGry = pGry + j, pCurSb = pSb + j; j<x2; j++, pCurGry++)
		{
			G1 = *(pCurGry - width - 1) + (*(pCurGry - width) * 2) + *(pCurGry - width + 1);
			G1 -= *(pCurGry + width - 1) + (*(pCurGry + width) * 2) + *(pCurGry + width + 1);

			G2 = *(pCurGry - 1 - width) + (*(pCurGry - 1) * 2) + *(pCurGry - 1 + width);
			G2 -= *(pCurGry + 1 - width) + (*(pCurGry + 1) * 2) + *(pCurGry + 1 + width);

			*(pCurSb++) = min(255, abs(G1) + abs(G2));
			//	*(pCurSb++)=abs(G1)+abs(G2);
		}
	}
	return;
}
