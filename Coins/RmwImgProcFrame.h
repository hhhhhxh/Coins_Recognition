class RmwImgProcFrame {
public:
	//构造/析构
	RmwImgProcFrame();
	~RmwImgProcFrame();
	//读入图像
	void init(char *s);
	//申请内存
	void ApplyCircle();
	//获取宽度和高度
	int GetWidth();
	int GetHeight();
	//获取半径
	int GetRadius();
	//设置采样倍数
	void SetRatioAndApply(int x);
	//把pImg缩小ratio倍，结果存入pHalfImg
	void ImageShrink();
	//对pHalfImg使用Sobel算子，结果存入pHalfSbImg
	void HalfSobel();
	//对pHalfSbImg做二值化，结果存入pHalfBinImg中
	void HalfBin();
	//对pHalfBinImg做圆的Hough Transform，得到圆心和半径
	//将x，y，r分别乘以ratio，得到原图中的圆心坐标和半径
	void HoughTransform();
	//将pImg中圆这一部分的图片抠出来放在pCircleImg中
	void GetCircleImage();
	//对pCircleImg进行Sobel算子，结果存入pCircleSbImg中
	void CircleSobel();
	//将原图中的硬币进行旋转，得到正的硬币，结果存入pRotImg
	void RotateImage(int alpha);
	//输出旋转后的图像
	void Output(char *s);
	//求pCircleSbImg中像素点的值
	int GetCircleSb(int x, int y);

private:
	// 内存释放
	void Dump();

private:
	//图像属性
	int m_width, m_height;
	int s_width, s_height;  // s_width = m_width / ratio
	int ratio; // 缩放倍数
	int x, y, r, tr; // 圆的系数, tr = 2 * r + 1
	//内存
	BYTE *m_pGryImg;
	BYTE *m_pHalfImg;
	BYTE *m_pHalfSbImg;
	BYTE *m_pHalfBinImg;
	BYTE *m_pCircleImg;
	BYTE *m_pCircleSbImg;
	BYTE *m_pRotImg;
	//图像的路径
	char *path;
};
RmwImgProcFrame::RmwImgProcFrame() {
	//<在构造函数中把所有的成员变量赋初值>
	//图像属性
	m_width = 0;
	m_height = 0;
	//内存空间
	m_pGryImg = NULL;
	m_pHalfImg = NULL;
	m_pHalfSbImg = NULL;
	m_pHalfBinImg = NULL;
	m_pCircleImg = NULL;
	m_pCircleSbImg = NULL;
}
RmwImgProcFrame::~RmwImgProcFrame()
{	//<在析构函数中释放所有的内存>
	Dump();
}
void RmwImgProcFrame::Dump()
{   //<写一个专门的用作释放内存的函数,因为内存会有多次的申请与释放>
	if (m_pGryImg) { delete m_pGryImg; m_pGryImg = NULL; }
	if (m_pHalfImg) { delete m_pHalfImg; m_pHalfImg = NULL; }
	if (m_pHalfSbImg) { delete m_pHalfSbImg; m_pHalfSbImg = NULL; }
	if (m_pHalfBinImg) { delete m_pHalfBinImg; m_pHalfBinImg = NULL; }
	if (m_pCircleImg) { delete m_pCircleImg; m_pCircleImg = NULL; }
	if (m_pCircleSbImg) { delete m_pCircleSbImg; m_pCircleSbImg = NULL; }
	if (path) { delete path; path = NULL; }
}
void RmwImgProcFrame::init(char *s) {
	Dump();
	path = new char[50];
	strcpy(path, s);
	m_pGryImg = RmwRead8BitBmpFile2Img(path, &m_width, &m_height);
	x = y = r = tr = -1;
}
void RmwImgProcFrame::ApplyCircle() {  // 申请圆图像的内存
	if (tr == -1)	return;
	m_pCircleImg = new BYTE[tr*tr];
	m_pCircleSbImg = new BYTE[tr*tr];
	m_pRotImg = new BYTE[tr*tr];
}
int RmwImgProcFrame::GetWidth() { //  返回图像的宽度
	return m_width;
}
int RmwImgProcFrame::GetHeight() { //  返回图像的高度
	return m_height;
}
void RmwImgProcFrame::SetRatioAndApply(int x) {
	ratio = x;
	s_width = m_width / ratio;
	s_height = m_height / ratio;
	m_pHalfImg = new BYTE[s_width*s_height];
	m_pHalfSbImg = new BYTE[s_width*s_height];
	m_pHalfBinImg = new BYTE[s_width*s_height];
}
void RmwImgProcFrame::ImageShrink() {
	HxhImageShrink(m_pHalfImg, m_pGryImg, m_width, m_height, ratio);
}
void RmwImgProcFrame::HalfSobel() {
	RmwSobelGrayImage(m_pHalfImg, s_width, s_height, m_pHalfSbImg);
}
void RmwImgProcFrame::HalfBin() {
	for (BYTE *pCur = m_pHalfSbImg; pCur < m_pHalfSbImg + s_width*s_height; pCur++) {
		*(m_pHalfBinImg + (pCur - m_pHalfSbImg)) = *pCur;
	}
	HxhBinary(m_pHalfBinImg, s_width, s_height, 100);
}
void RmwImgProcFrame::HoughTransform() {
	CircleHough(m_pHalfBinImg, s_width, s_height, x, y, r);
	x *= ratio;
	y *= ratio;
	r *= ratio;
	tr = 2 * r + 1;
}
void RmwImgProcFrame::GetCircleImage() {
	HxhGetCircle(m_pGryImg, m_pCircleImg, m_width, m_height, x, y, r);
}
void RmwImgProcFrame::CircleSobel() {
	RmwSobelGrayImage(m_pCircleImg, tr, tr, m_pCircleSbImg);
}
int RmwImgProcFrame::GetRadius() {
	return r;
}
void RmwImgProcFrame::RotateImage(int alpha) {
	HxhGetRotImage(m_pCircleImg, m_pRotImg, tr, tr, r, r, r, alpha);
}
void RmwImgProcFrame::Output(char *s) {
	RmwWrite8BitImg2BmpFile(m_pRotImg, tr, tr, s);
}
int RmwImgProcFrame::GetCircleSb(int x, int y) {
	return *(m_pCircleSbImg + y*tr + x); //pCircleSbImg + (tr - ty)*tr + tx
}