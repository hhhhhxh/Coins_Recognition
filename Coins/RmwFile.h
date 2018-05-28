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
	//�����һ��
	pGry = pGryImg;
	pSum = pSumImg;
	*(pSum++) = *(pGry++);
	for (x = 1; x < width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	//����������
	for (y = 1; y < height; y++) {
		*(pSum++) = *(pSum - width) + (*(pGry++)); //��һ��
		for (x = 1; x < width; x++) { //������
			*(pSum++) = *(pGry++) + (*(pSum - 1)) + (*(pSum - width)) - (*(pSum - width - 1));
		}
	}
}