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
	//      ��ȡ��׼Ӳ�ҵ�����
	//      0, 1 --> 1��������
	//      2, 3 --> 5��������
	//      4, 5 --> 1Ԫ������
	/////////////////////////////////////////////

	memset(eigen, 0, sizeof(eigen));
	memset(avg, 0, sizeof(avg));

	for (int cnt = 0; cnt < 6; cnt++)
	{
		/////////////////////////////////////
		// �����׼ͼ�񣬴���pImg
		/////////////////////////////////////
		sprintf_s(StandardFilePath, "./standard/%d.bmp", cnt);
		int width = 0, height = 0;
		BYTE *pImg = RmwRead8BitBmpFile2Img(StandardFilePath, &width, &height);

		//////////////////////////////////////////////
		// ��pImg��ͼ��Ӧ��Sobel���ӣ��������pSbImg
		//////////////////////////////////////////////
		BYTE *pSbImg = new BYTE[width * height];
		RmwSobelGrayImage(pImg, width, height, pSbImg);

		//////////////////////////////////////////////
		// ��pSbImg���ж�ֵ�����������pBinImg
		//////////////////////////////////////////////
		BYTE *pBinImg = new BYTE[width * height];
		for (BYTE *pCur = pSbImg; pCur < pSbImg + width * height; pCur++)
		{
			*(pBinImg + (pCur - pSbImg)) = *pCur;
		}
		HxhBinary(pBinImg, width, height, 100);

		/////////////////////////////////////////////////////
		// ��pBinImg��Բ��Hough Transform���õ�Բ�ĺͰ뾶
		/////////////////////////////////////////////////////
		int x = 0, y = 0, r = 0;
		int *Countx = new int[width];
		int *County = new int[height];
		CircleHough(pBinImg, width, height, x, y, r);
		// printf("Circle Center: x = %d  y = %d  r = %d\n", x, y, r);

		//////////////////////////////////////////////////////////////////////////
		// eigen�����¼��pSbImg�а뾶Ϊr*v1��r*v2֮���Բ����ÿ���Ƕȵ����غ�
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
	// avg[cnt]��¼�˵�cnt�ű�׼ͼ����ÿ���Ƕ����غ͵�ƽ��ֵ
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
	// �����������Ϊ2
	/////////////////////////
	Img.SetRatioAndApply(2);

	////////////////////////////////////////////
	// ��pImg��Сratio�����������pHalfImg
	////////////////////////////////////////////
	Img.ImageShrink();

	////////////////////////////////////////////////
	// ��pHalfImgʹ��Sobel���ӣ��������pHalfSbImg
	////////////////////////////////////////////////
	Img.HalfSobel();

	////////////////////////////////////////////////
	// ��pHalfSbImg����ֵ�����������pHalfBinImg��
	////////////////////////////////////////////////
	Img.HalfBin();

	///////////////////////////////////////////////////////
	// ��pHalfBinImg��Բ��Hough Transform���õ�Բ�ĺͰ뾶
	///////////////////////////////////////////////////////
	Img.HoughTransform();

	//////////////////////////////////////////////////////////
	// ��pImg��Բ��һ���ֵ�ͼƬ�ٳ�������pCircleImg��
	// ��СΪ(2 * r + 1) * (2 * r + 1)
	//////////////////////////////////////////////////////////
	Img.ApplyCircle();
	Img.GetCircleImage();

	//////////////////////////////////////////////////////////
	// ��pCircleImg����Sobel���ӣ��������pCircleSbImg��
	//////////////////////////////////////////////////////////
	Img.CircleSobel();

	//////////////////////////////////////////////////////
	// ��pCircleSbImg�л�ȡ��ǰͼ�������ֵ��ƽ������ֵ
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
	// ���ݰ뾶�Ĵ�С����Ӳ�ҵ���ֵ
	///////////////////////////////////////
	double m = -1; // ��ֵ
	if (r >= 101)
		m = 1.0;
	else if (r <= 87)
		m = 0.1;
	else
		m = 0.5;

	///////////////////////////////////////////////////////////////////////////////////////
	// ���ݵ�ǰӲ�ҵ�����ֵ���׼Ӳ������ֵ�����ϵ�������ֵ�ǰӲ�ҵ������棬���ó���ת�Ƕ�
	// ���У�rhoΪ���ϵ�������ֵ��idxΪȡ�����ֵʱ���±꣬alphaΪȡ�����ֵʱ��Ҫ��ת�ĽǶ�
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
	// ��ԭͼ�е�Ӳ�ҽ�����ת���õ�����Ӳ�ң��������pRotImg
	////////////////////////////////////////////////////////
	Img.RotateImage(alpha);

	////////////////////////////////////////////////////////
	// ���ת�����Ӳ��ͼ�񣬲�����Ӳ�ҵ���ֵ��������
	////////////////////////////////////////////////////////
	if (idx % 2 == 0)
		sprintf_s(AnsPath, "./ans/_����_%.1fԪ_��ת%d��.bmp", m, alpha);
	else
		sprintf_s(AnsPath, "./ans/_����_%.1fԪ_��ת%d��.bmp", m, alpha);
	Img.Output(AnsPath);
}

int main()
{
	/////////////////////////////////
	// Ԥ����sin,cos����
	/////////////////////////////////
	for (int i = 0; i < 360; i++)
	{
		Hxhsin[i] = (int)(sin((double)i * PI / 180.0) * 2048);
		Hxhcos[i] = (int)(cos((double)i * PI / 180.0) * 2048);
	}

	/////////////////////////////////
	// ��ȡ��׼Ӳ�ҵ�����
	/////////////////////////////////
	getStandardCoins();
	printf("Got the Data of Standard Coins.\n");

	///////////////////////////////////////////
	// �����׼ͼ�������ֵ��eigen.txt�ļ���
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
	if (in) // �и��ļ�
	{
		clock_t t1 = clock();
		while (getline(in, line_in)) // line�в�����ÿ�еĻ��з�
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