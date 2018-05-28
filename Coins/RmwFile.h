#include <windows.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void RmwDoSumGryImg(BYTE *pGryImg, int width, int height, int *pSumImg) {
	BYTE *pGry;
	int *pSum;
	int x, y;
	//先求第一行
	pGry = pGryImg;
	pSum = pSumImg;
	*(pSum++) = *(pGry++);
	for (x = 1; x < width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	//再求其他行
	for (y = 1; y < height; y++) {
		*(pSum++) = *(pSum - width) + (*(pGry++)); //第一列
		for (x = 1; x < width; x++) { //其他列
			*(pSum++) = *(pGry++) + (*(pSum - 1)) + (*(pSum - width)) - (*(pSum - width - 1));
		}
	}
}