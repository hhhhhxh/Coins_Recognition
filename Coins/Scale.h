#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

void HxhImageShrink(BYTE *pAnsImg, BYTE *pImg, int width, int height, int scale) {
	// scale为缩放因子
	int x = 0, y = 0, ratio = scale;
	BYTE *pCur1 = pImg, *pCur2 = pAnsImg;
	for (x = 0; x < height; x += ratio) {
		for (y = 0; y < width; y += ratio) {
			*(pCur2++) = (*pCur1);
			pCur1 += ratio;
		}
		pCur1 += (ratio - 1)*width;
	}
}