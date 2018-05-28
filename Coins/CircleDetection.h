#include <windows.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>

double dist(int x1, int y1, int x2, int y2) {
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

void CircleHough(BYTE *pImg, int width, int height, int &MAX_x, int &MAX_y, int &MAX_r) {
	int *Countx = new int[width];
	int *County = new int[height];
	memset(Countx, 0, sizeof(int) * width);
	memset(County, 0, sizeof(int) * height);
	// Get Countx
	for (int y = 0; y < height; y++) {
		int tmp1 = -1, tmp2 = -1;
		for (int x = 0; x < width; x++) {
			if (*(pImg + y*width + x) == 255) {
				tmp1 = x;
				break;
			}
		}
		for (int x = width - 1; x >= 0; x--) {
			if (*(pImg + y*width + x) == 255) {
				tmp2 = x;
				break;
			}
		}
		if (tmp1 == -1 || tmp2 == -1)	continue;
		Countx[(tmp1 + tmp2) / 2]++;
	}

	// Get County
	for (int x = 0; x < width; x++) {
		int tmp1 = -1, tmp2 = -1;
		for (int y = 0; y < height; y++) {
			if (*(pImg + y*width + x) == 255) {
				tmp1 = y;
				break;
			}
		}
		for (int y = height - 1; y >= 0; y--) {
			if (*(pImg + y*width + x) == 255) {
				tmp2 = y;
				break;
			}
		}
		if (tmp1 == -1 || tmp2 == -1)	continue;
		County[(tmp1 + tmp2) / 2]++;
	}

	MAX_x = MAX_y = MAX_r = -1;
	int MAX = -1;
	for (int x = 0; x < width; x++) {
		if (Countx[x] > MAX) {
			MAX = Countx[x];
			MAX_x = x;
		}
	}
	MAX = -1;
	for (int y = 0; y < height; y++) {
		if (County[y] > MAX) {
			MAX = County[y];
			MAX_y = y;
		}
	}
	int maxr = (int)sqrt(width*width + height*height) + 1;
	int *r = new int[maxr];
	memset(r, 0, sizeof(int) * maxr);
	BYTE *pCur = pImg;
	MAX = -1;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (*pCur == 255) {
				int tmpr = (int)dist(x, y, MAX_x, MAX_y);
				r[tmpr]++;
				if (r[tmpr] > MAX) {
					MAX = r[tmpr];
					MAX_r = tmpr;
				}
			}
			pCur++;
		}
	}

	delete Countx;
	delete County;
	delete r;
}

void HxhGetCircle(BYTE *pImg, BYTE *pNewImg, int width, int height, int x, int y, int r) {
	BYTE *pCur = pNewImg;
	for (int i = y - r; i <= y + r; i++) {
		for (int j = x - r; j <= x + r; j++) {
			*(pCur++) = *(pImg + i*width + j);
		}
	}
}

//const double PI = acos(-1.0);

void HxhDrawCircle(BYTE *pImg, int width, int height, int x, int y, int r) {
	int Hxhsin[360];
	int Hxhcos[360];
	for (int i = 0; i < 360; i++) {
		Hxhsin[i] = (int)(sin((double)i*PI / 180.0) * 2048);
		Hxhcos[i] = (int)(cos((double)i*PI / 180.0) * 2048);
	}
	x *= 2048;
	y *= 2048;
	for (int i = 0; i < 360; i++) {
		int tx = (x + r*Hxhcos[i]) >> 11;
		int ty = (y + r*Hxhsin[i]) >> 11;
		//		printf("tx = %d ty = %d\n", tx, ty);
		if (tx < 0 || tx >= width || ty < 0 || ty >= height)	continue;
		*(pImg + ty*width + tx) = 255;
	}
}