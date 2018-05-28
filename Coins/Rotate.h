#include <windows.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;
	
int dist2(int x1, int y1, int x2, int y2) {
	return (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
}

void HxhGetRotImage(BYTE *pImg, BYTE *pRotImg, int width, int height, 
										int rx, int ry, int r, int theta) {
	// pImg和pRotImg大小都是width*height
	double cosV = cos(theta*PI / 180.0);
	double sinV = sin(theta*PI / 180.0);
	BYTE *pCur = pImg;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (dist2(x, y, r, r) > r*r) {
				*(pRotImg + y*width + x) = *(pCur++);
				continue;
			}
			// 在旋转后图像中坐标为(xx, yy)的点（以(r, r)为圆心）
			int xx = x - r;
			int yy = r - y;
			// 这个点在原图中坐标为(tx, ty)
			double tx = xx*cosV - yy*sinV + 0.00001;
			double ty = xx*sinV + yy*cosV + 0.00001;
			// 求出上下界并转换到图中的实际坐标
			int tx_floor = floor(tx);
			int tx_ceil = ceil(tx);
			int ty_floor = floor(ty);
			int ty_ceil = ceil(ty);
			tx_floor += r;
			tx_ceil += r;
			ty_floor = r - ty_floor;
			ty_ceil = r - ty_ceil;
			tx += r;
			ty = r - ty;
			if (tx_floor > tx_ceil)	swap(tx_floor, tx_ceil);
			if (ty_floor > ty_ceil)	swap(ty_floor, ty_ceil);
			double tmp1 = ((tx_ceil - tx)*(*(pImg + ty_ceil*width + tx_floor))
				+ (tx - tx_floor)*(*(pImg + ty_ceil*width + tx_ceil))) / (tx_ceil - tx_floor);
			double tmp2 = ((tx_ceil - tx)*(*(pImg + ty_floor*width + tx_floor))
				+ (tx - tx_floor)*(*(pImg + ty_floor*width + tx_ceil))) / (tx_ceil - tx_floor);
			*(pRotImg + y*width + x) = ((ty_floor-ty)*tmp1+(ty-ty_ceil)*tmp2) / (ty_floor-ty_ceil);
			pCur++;
		}
	}
}