#include <windows.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void HxhBinary(BYTE *pImg, int width, int height, int threshold) {
	for (BYTE *pCur = pImg; pCur < pImg + width*height; pCur++) {
		if (*pCur >= threshold)	
			*pCur = 255;
		else	
			*pCur = 0;
	}
}

void HxhOtsuBinary(BYTE *pImg, int width, int height) {
	int hist[256];
	memset(hist, 0, sizeof(hist));
	for (BYTE *pCur = pImg; pCur < pImg + width*height; pCur++)	hist[*pCur]++;
	double pixelRatio[256];
	for (int i = 0; i < 256; i++) {
		pixelRatio[i] = (double)hist[i] / (double)(width*height);
	}  
	int threshold = 0;
	double w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
	for (int i = 0; i < 256; i++) {
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
		for (int j = 0; j < 256; j++) {
			if (j <= i) {
				w0 += pixelRatio[j];
				u0tmp += j * pixelRatio[j];
			}
			else {
				w1 += pixelRatio[j];
				u1tmp += j * pixelRatio[j];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		u = u0tmp + u1tmp;
		deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
		if (deltaTmp > deltaMax) {
			deltaMax = deltaTmp;
			threshold = i;
		}
	}
	for (BYTE *pCur = pImg; pCur < pImg + width*height; pCur++) {
		*pCur = ((*pCur) >= threshold) ? 255 : 0;
	}
}