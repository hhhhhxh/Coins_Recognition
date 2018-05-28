class RmwImgProcFrame {
public:
	//����/����
	RmwImgProcFrame();
	~RmwImgProcFrame();
	//����ͼ��
	void init(char *s);
	//�����ڴ�
	void ApplyCircle();
	//��ȡ��Ⱥ͸߶�
	int GetWidth();
	int GetHeight();
	//��ȡ�뾶
	int GetRadius();
	//���ò�������
	void SetRatioAndApply(int x);
	//��pImg��Сratio�����������pHalfImg
	void ImageShrink();
	//��pHalfImgʹ��Sobel���ӣ��������pHalfSbImg
	void HalfSobel();
	//��pHalfSbImg����ֵ�����������pHalfBinImg��
	void HalfBin();
	//��pHalfBinImg��Բ��Hough Transform���õ�Բ�ĺͰ뾶
	//��x��y��r�ֱ����ratio���õ�ԭͼ�е�Բ������Ͱ뾶
	void HoughTransform();
	//��pImg��Բ��һ���ֵ�ͼƬ�ٳ�������pCircleImg��
	void GetCircleImage();
	//��pCircleImg����Sobel���ӣ��������pCircleSbImg��
	void CircleSobel();
	//��ԭͼ�е�Ӳ�ҽ�����ת���õ�����Ӳ�ң��������pRotImg
	void RotateImage(int alpha);
	//�����ת���ͼ��
	void Output(char *s);
	//��pCircleSbImg�����ص��ֵ
	int GetCircleSb(int x, int y);

private:
	// �ڴ��ͷ�
	void Dump();

private:
	//ͼ������
	int m_width, m_height;
	int s_width, s_height;  // s_width = m_width / ratio
	int ratio; // ���ű���
	int x, y, r, tr; // Բ��ϵ��, tr = 2 * r + 1
	//�ڴ�
	BYTE *m_pGryImg;
	BYTE *m_pHalfImg;
	BYTE *m_pHalfSbImg;
	BYTE *m_pHalfBinImg;
	BYTE *m_pCircleImg;
	BYTE *m_pCircleSbImg;
	BYTE *m_pRotImg;
	//ͼ���·��
	char *path;
};
RmwImgProcFrame::RmwImgProcFrame() {
	//<�ڹ��캯���а����еĳ�Ա��������ֵ>
	//ͼ������
	m_width = 0;
	m_height = 0;
	//�ڴ�ռ�
	m_pGryImg = NULL;
	m_pHalfImg = NULL;
	m_pHalfSbImg = NULL;
	m_pHalfBinImg = NULL;
	m_pCircleImg = NULL;
	m_pCircleSbImg = NULL;
}
RmwImgProcFrame::~RmwImgProcFrame()
{	//<�������������ͷ����е��ڴ�>
	Dump();
}
void RmwImgProcFrame::Dump()
{   //<дһ��ר�ŵ������ͷ��ڴ�ĺ���,��Ϊ�ڴ���ж�ε��������ͷ�>
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
void RmwImgProcFrame::ApplyCircle() {  // ����Բͼ����ڴ�
	if (tr == -1)	return;
	m_pCircleImg = new BYTE[tr*tr];
	m_pCircleSbImg = new BYTE[tr*tr];
	m_pRotImg = new BYTE[tr*tr];
}
int RmwImgProcFrame::GetWidth() { //  ����ͼ��Ŀ��
	return m_width;
}
int RmwImgProcFrame::GetHeight() { //  ����ͼ��ĸ߶�
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