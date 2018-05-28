#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
using namespace std;

const double PI = acos(-1.0);

#include "bmpFile.h"
#include "RmwFile.h"
#include "CircleDetection.h"
#include "Binarization.h"
#include "Rotate.h"
#include "Sobel.h"
#include "Scale.h"
#include "RmwImgProcFrame.h"

//#define EIGEN_OUTPUT

char StandardFilePath[50];
char ReadFilePath[50];
char AnsPath[50];

double eigen[360][6];
double avg[6];
double v1 = 0.7, v2 = 0.9;

int Hxhsin[360], Hxhcos[360];

void getStandardCoins()
{
	/////////////////////////////////////////////
	//      获取标准硬币的数据
	//      0, 1 --> 1角正、反
	//      2, 3 --> 5角正、反
	//      4, 5 --> 1元正、反
	/////////////////////////////////////////////

	memset(eigen, 0, sizeof(eigen));
	memset(avg, 0, sizeof(avg));

	for (int cnt = 0; cnt < 6; cnt++)
	{
		/////////////////////////////////////
		// 输入标准图像，存入pImg
		/////////////////////////////////////
		sprintf_s(StandardFilePath, "./standard/%d.bmp", cnt);
		int width = 0, height = 0;
		BYTE *pImg = RmwRead8BitBmpFile2Img(StandardFilePath, &width, &height);

		//////////////////////////////////////////////
		// 对pImg的图像应用Sobel算子，结果存入pSbImg
		//////////////////////////////////////////////
		BYTE *pSbImg = new BYTE[width * height];
		RmwSobelGrayImage(pImg, width, height, pSbImg);

		//////////////////////////////////////////////
		// 对pSbImg进行二值化，结果存入pBinImg
		//////////////////////////////////////////////
		BYTE *pBinImg = new BYTE[width * height];
		for (BYTE *pCur = pSbImg; pCur < pSbImg + width * height; pCur++)
		{
			*(pBinImg + (pCur - pSbImg)) = *pCur;
		}
		HxhBinary(pBinImg, width, height, 100);

		/////////////////////////////////////////////////////
		// 对pBinImg做圆的Hough Transform，得到圆心和半径
		/////////////////////////////////////////////////////
		int x = 0, y = 0, r = 0;
		int *Countx = new int[width];
		int *County = new int[height];
		CircleHough(pBinImg, width, height, x, y, r);
		// printf("Circle Center: x = %d  y = %d  r = %d\n", x, y, r);

		//////////////////////////////////////////////////////////////////////////
		// eigen数组记录了pSbImg中半径为r*v1和r*v2之间的圆环上每个角度的像素和
		//////////////////////////////////////////////////////////////////////////
		for (int i = 0; i < 360; i++)
		{
			for (double rr = r * v1; rr <= r * v2; rr++)
			{
				int tx = (int)(x * 2048 + rr * Hxhcos[i]) >> 11;
				int ty = (int)((height - y) * 2048 + rr * Hxhsin[i]) >> 11;
				eigen[i][cnt] += *(pSbImg + (height - ty) * width + tx);
			}
		}
	}

	/////////////////////////////////////////////////////////////
	// avg[cnt]记录了第cnt张标准图像上每个角度像素和的平均值
	/////////////////////////////////////////////////////////////
	for (int cnt = 0; cnt < 6; cnt++)
	{
		for (int i = 0; i < 360; i++)
		{
			avg[cnt] += eigen[i][cnt];
		}
		avg[cnt] /= 360.0;
	}
}

void work(char *path)
{
	int width = 0, height = 0;
	RmwImgProcFrame Img;

	Img.init(path);
	printf("Read Completed. <%d * %d>\n", Img.GetWidth(), Img.GetHeight());

	/////////////////////////
	// 定义采样倍数为2
	/////////////////////////
	Img.SetRatioAndApply(2);

	////////////////////////////////////////////
	// 把pImg缩小ratio倍，结果存入pHalfImg
	////////////////////////////////////////////
	Img.ImageShrink();

	////////////////////////////////////////////////
	// 对pHalfImg使用Sobel算子，结果存入pHalfSbImg
	////////////////////////////////////////////////
	Img.HalfSobel();

	////////////////////////////////////////////////
	// 对pHalfSbImg做二值化，结果存入pHalfBinImg中
	////////////////////////////////////////////////
	Img.HalfBin();

	///////////////////////////////////////////////////////
	// 对pHalfBinImg做圆的Hough Transform，得到圆心和半径
	///////////////////////////////////////////////////////
	Img.HoughTransform();

	//////////////////////////////////////////////////////////
	// 将pImg中圆这一部分的图片抠出来放在pCircleImg中
	// 大小为(2 * r + 1) * (2 * r + 1)
	//////////////////////////////////////////////////////////
	Img.ApplyCircle();
	Img.GetCircleImage();

	//////////////////////////////////////////////////////////
	// 对pCircleImg进行Sobel算子，结果存入pCircleSbImg中
	//////////////////////////////////////////////////////////
	Img.CircleSobel();

	//////////////////////////////////////////////////////
	// 从pCircleSbImg中获取当前图像的特征值和平均特征值
	//////////////////////////////////////////////////////
	double CurEigen[360];
	double CurAvg = 0;
	int r = Img.GetRadius();
	int tr = 2 * r + 1;
	memset(CurEigen, 0, sizeof(CurEigen));
	for (int i = 0; i < 360; i++)
	{
		for (double rr = r * v1; rr <= r * v2; rr++)
		{
			int tx = (int)(r * 2048 + rr * Hxhcos[i]) >> 11;
			int ty = (int)((tr - r) * 2048 + rr * Hxhsin[i]) >> 11;
			CurEigen[i] += Img.GetCircleSb(tx, tr - ty);
		}
	}
	for (int i = 0; i < 360; i++)
		CurAvg += CurEigen[i];
	CurAvg /= 360.0;

	///////////////////////////////////////
	// 根据半径的大小区分硬币的面值
	///////////////////////////////////////
	double m = -1; // 面值
	if (r >= 101)
		m = 1.0;
	else if (r <= 87)
		m = 0.1;
	else
		m = 0.5;

	///////////////////////////////////////////////////////////////////////////////////////
	// 根据当前硬币的特征值与标准硬币特征值的相关系数来区分当前硬币的正反面，并得出旋转角度
	// 其中，rho为相关系数的最大值，idx为取到最大值时的下标，alpha为取到最大值时所要旋转的角度
	///////////////////////////////////////////////////////////////////////////////////////
	double rho = -2.0;
	int idx = 0, alpha = 0;
	for (int k = 0; k < 360; k++)
	{
		int l, r;
		if (m == 0.1)
			l = 0, r = 1;
		if (m == 0.5)
			l = 2, r = 3;
		if (m == 1)
			l = 4, r = 5;
		for (int id = l; id <= r; id++)
		{
			double tmp, tmp1, tmp2, tmp3;
			tmp = tmp1 = tmp2 = tmp3 = 0;
			for (int i = 0; i < 360; i++)
			{
				tmp1 += (CurEigen[(i + k) % 360] - CurAvg) * (eigen[i][id] - avg[id]);
				tmp2 += (CurEigen[(i + k) % 360] - CurAvg) * (CurEigen[(i + k) % 360] - CurAvg);
				tmp3 += (eigen[i][id] - avg[id]) * (eigen[i][id] - avg[id]);
			}
			tmp = tmp1 / sqrt(tmp2 * tmp3);
			if (tmp > rho)
			{
				rho = tmp;
				idx = id;
				alpha = k;
			}
		}
	}

	////////////////////////////////////////////////////////
	// 将原图中的硬币进行旋转，得到正的硬币，结果存入pRotImg
	////////////////////////////////////////////////////////
	Img.RotateImage(alpha);

	////////////////////////////////////////////////////////
	// 输出转正后的硬币图像，并标明硬币的面值与正反面
	////////////////////////////////////////////////////////
	if (idx % 2 == 0)
		sprintf_s(AnsPath, "./ans/_正面_%.1f元_旋转%d度.bmp", m, alpha);
	else
		sprintf_s(AnsPath, "./ans/_反面_%.1f元_旋转%d度.bmp", m, alpha);
	Img.Output(AnsPath);
}

int main()
{
	/////////////////////////////////
	// 预处理sin,cos数组
	/////////////////////////////////
	for (int i = 0; i < 360; i++)
	{
		Hxhsin[i] = (int)(sin((double)i * PI / 180.0) * 2048);
		Hxhcos[i] = (int)(cos((double)i * PI / 180.0) * 2048);
	}

	/////////////////////////////////
	// 获取标准硬币的数据
	/////////////////////////////////
	getStandardCoins();
	printf("Got the Data of Standard Coins.\n");

	///////////////////////////////////////////
	// 输出标准图像的特征值到eigen.txt文件中
	///////////////////////////////////////////
#ifdef EIGEN_OUTPUT
	FILE *stream = fopen("eigen.txt", "w");
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 360; j++)
		{
			fprintf(stream, "%.2f ", eigen[j][i]);
		}
		fprintf(stream, "\n\n");
	}
	fclose(stream);
#endif

	ifstream in;
	const char *path_in;
	char *path;
	string line_in;
	in.open(".\\src\\bmpFileNameList.txt");
	if (in) // 有该文件
	{
		clock_t t1 = clock();
		while (getline(in, line_in)) // line中不包括每行的换行符
		{
			path_in = line_in.c_str();
			path = const_cast<char *>(path_in);
			printf("%s\n", path);
			work(path);
		}
		clock_t t2 = clock();
		printf("time = %.fms\n", (t2 - t1) / 96.0);
		system("pause");
	}
	in.close();
}