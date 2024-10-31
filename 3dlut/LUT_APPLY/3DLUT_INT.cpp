//#include <iostream>
//#include <iomanip>
//
//#include <string.h>
//#include <vector>
//#include <fstream>
//#include <iostream>
//#include <string>
//#include <cmath>
//
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui_c.h>
//
//using namespace std;
//using namespace cv;
//
//#define  Num  33   //查找表取值空间是32*32*32，
//#define  Invalid_NumberRows 5  //a.cube = 12
//#define  shift 20	//if not
//#define  cube_shift 10			//cube的两个相邻点之间的精度
////#define  _CRT_SECURE_NO_WARNINGS
//
////int YUV2RGB(int r, int g, int b) {
////	int YUV[3] = { 0 };
////	return YUV;
////}
////
////int RGB2YUV(int r, int g, int b) {
////
////}
//
//int cubeMerge(int cctA, int cctB, int cctNow, int LutAValue, int LutBValue) {
//	int LutMergeValue = 0;
//
//	return LutMergeValue;
//}
//
//int cubeIndex(int r, int g, int b)
//{
//	return (int(r + g * Num + b * Num * Num));
//}
//
//int mixvalue(int a, int b, int c)
//{
//	int tmp = pow(2, shift);
//	//int tmp2 = c >> 22;
//	int c2 = c % tmp;
//	int delta = ((b - a)* c2) >> shift;
//	return (a + delta);
//}
//
//int floorINT(int x)
//{
//	return x >> shift;
//}
//
//int ceilINT(int x)
//{
//	int x1 = x >> shift;
//	int a = pow(2, shift);
//	int x2 = x % a;
//
//	return x2 > 0 ? (x1 + 1) : x1;
//}
//
//template <typename T>
//struct ColorTriplet
//{
//	T R;
//	T G;
//	T B;
//};
//
//int InterpolateHelper(int v0, int v1, int f)
//{
//	int delta = ((v1 - v0) * f + (1<<(shift -1))) >> shift ;	//四舍五入
//	return v0 + delta;
//}
//
//ColorTriplet<int> Interpolate(ColorTriplet<int>& v0, ColorTriplet<int>& v1, int f)
//{
//	int tmp = pow(2, shift); //直接减右移的值 或者直接与
//	//int tmp2 = c >> 22;
//	int delta = f % tmp;
//
//	ColorTriplet<int> out = {
//		InterpolateHelper(v0.R, v1.R, delta),
//		InterpolateHelper(v0.G, v1.G, delta),
//		InterpolateHelper(v0.B, v1.B, delta)
//	};
//
//	return out;
//}
//
//int main()
//{
//	//step1 读取*.cube文件
//
//	char* text, * point_ext;
//	FILE* pf = fopen("D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\outputInt_e4.cube", "rb+");
//	if (pf == NULL)
//	{
//		printf("error!");
//		return -1;
//	}
//
//	fseek(pf, 0, SEEK_END);
//	long lSize = ftell(pf);
//
//	text = new char[lSize];
//	rewind(pf);
//
//	fread(text, sizeof(char), lSize, pf);
//	char* char_w = new char[8];
//	int LUTD3[Num * Num * Num * 3] = { 0 };
//	//int LUTD3_1[Num * Num * Num * 3] = { 0 };
//
//	int t(0), i(0), itN(0), invat(0);
//
//	//定位到有效数据的起始位置
//	point_ext = text;
//
//	for (i = 8; i <= lSize; i+=8)
//	{
//		memcpy(char_w, point_ext + i - 8, 8);
//		LUTD3[t++] = atoi(char_w);
//		//LUTD3_1[t] = 12;
//		//printf("%.6d\n", LUTD3[t-1]);
//		memset(char_w, 0, 8);
//
//	}
//
//	// step2 读取图像
//
//	Mat ORI = imread("D:\\work\\3dlut\\a.jpg");  //1234.bmp/cat.jpg/detailIM.jpg
//	namedWindow("原图像", CV_WINDOW_NORMAL);
//	imshow("原图像", ORI);
//
//	// step3 3D_LUT arith
//
//	Mat ORI_normfloat = Mat(ORI.size(), CV_32FC3);
//	Mat NewIM = ORI.clone();
//
//	int toR, toG, toB;
//	unsigned char rH, gH, bH, rL, gL, bL;
//	int index000, index001,index010, index011, index100, index101, index110, index111;
//
//	for (int i = 0; i < ORI.rows; i++)
//	{
//		for (int j = 0; j < ORI.cols; j++)
//		{
//
//			int r255 = (ORI.at<Vec3b>(i, j)[2]);
//			int g255 = (ORI.at<Vec3b>(i, j)[1]);
//			int b255 = (ORI.at<Vec3b>(i, j)[0]);
//
//			//r255 = 250;
//			//g255 = 250;
//			//b255 = 250;
//			//printf("r255:%.6d\n", r255);
//			int r255_2 = r255 << shift;
//			int g255_2 = g255 << shift;
//			int b255_2 = b255 << shift;
//			//printf("r255_2:%.6d\n", r255_2);
//			int r255_3 = r255_2 / 255 * (Num - 1);			//直接右移的方法得到ceil和floor
//			int g255_3 = g255_2 / 255 * (Num - 1);
//			int b255_3 = b255_2 / 255 * (Num - 1);
//			//printf("r255_3:%.6d\n", r255_3);
//			rH = ceilINT(r255_3);
//			gH = ceilINT(g255_3);
//			bH = ceilINT(b255_3);
//
//			rL = floorINT(r255_3);
//			gL = floorINT(g255_3);
//			bL = floorINT(b255_3);
//
//			index000 = cubeIndex(rL, gL, bL);
//			index001 = cubeIndex(rL, gL, bH);
//			index010 = cubeIndex(rL, gH, bL);
//			index011 = cubeIndex(rL, gH, bH);
//			index100 = cubeIndex(rH, gL, bL);
//			index101 = cubeIndex(rH, gL, bH);
//			index110 = cubeIndex(rH, gH, bL);
//			index111 = cubeIndex(rH, gH, bH);
//
//			ColorTriplet<int> c000 = { LUTD3[index000 * 3], LUTD3[index000 * 3 + 1], LUTD3[index000 * 3 + 2] };
//			ColorTriplet<int> c001 = { LUTD3[index001 * 3], LUTD3[index001 * 3 + 1], LUTD3[index001 * 3 + 2] };
//			ColorTriplet<int> c010 = { LUTD3[index010 * 3], LUTD3[index010 * 3 + 1], LUTD3[index010 * 3 + 2] };
//			ColorTriplet<int> c011 = { LUTD3[index011 * 3], LUTD3[index011 * 3 + 1], LUTD3[index011 * 3 + 2] };
//			ColorTriplet<int> c100 = { LUTD3[index100 * 3], LUTD3[index100 * 3 + 1], LUTD3[index100 * 3 + 2] };
//			ColorTriplet<int> c101 = { LUTD3[index101 * 3], LUTD3[index101 * 3 + 1], LUTD3[index101 * 3 + 2] };
//			ColorTriplet<int> c110 = { LUTD3[index110 * 3], LUTD3[index110 * 3 + 1], LUTD3[index110 * 3 + 2] };
//			ColorTriplet<int> c111 = { LUTD3[index111 * 3], LUTD3[index111* 3 + 1], LUTD3[index111 * 3 + 2] };
//
//			// c00 -> interpolate c000 and c100
//			ColorTriplet<int> c00 = Interpolate(c000, c100, r255_3);
//			// c01 -> interpolate c001 and c101
//			ColorTriplet<int> c01 = Interpolate(c001, c101, r255_3);
//			// c10 -> interpolate c010 and c110
//			ColorTriplet<int> c10 = Interpolate(c010, c110, r255_3);
//			// c11 -> interpolate c011 and c111
//			ColorTriplet<int> c11 = Interpolate(c011, c111, r255_3);
//			// c0 -> interpolate c00 and c10
//			ColorTriplet<int> c0 = Interpolate(c00, c10, g255_3);
//			// c1 -> interpolate c01 and c11
//			ColorTriplet<int> c1 = Interpolate(c01, c11, g255_3);
//			// c -> interpolate c0 and c1
//			ColorTriplet<int> c = Interpolate(c0, c1, b255_3);
//
//			NewIM.at<Vec3b>(i, j)[0] = uchar(c.B);
//			NewIM.at<Vec3b>(i, j)[1] = uchar(c.G);
//			NewIM.at<Vec3b>(i, j)[2] = uchar(c.R);
//		}
//	}
//
//	//int toR, toG, toB;
//	//unsigned char rH, gH, bH, rL, gL, bL;
//	//int indexH, indexL;
//
//	//for (int i = 0; i < ORI.rows; i++)
//	//{
//	//	for (int j = 0; j < ORI.cols; j++)
//	//	{
//
//	//		int r255 = (ORI.at<Vec3b>(i, j)[2]);
//	//		int g255 = (ORI.at<Vec3b>(i, j)[1]);
//	//		int b255 = (ORI.at<Vec3b>(i, j)[0]);
//
//	//		//r255 = 255;
//	//		//g255 = 255;
//	//		//b255 = 255;
//	//		//printf("r255:%.6d\n", r255);
//	//		int r255_2 = r255 << shift;
//	//		int g255_2 = g255 << shift;
//	//		int b255_2 = b255 << shift;
//	//		//printf("r255_2:%.6d\n", r255_2);
//	//		int r255_3 = r255_2 / 255 * (Num - 1);
//	//		int g255_3 = g255_2 / 255 * (Num - 1);
//	//		int b255_3 = b255_2 / 255 * (Num - 1);
//	//		//printf("r255_3:%.6d\n", r255_3);
//	//		rH = ceilINT(r255_3);
//	//		gH = ceilINT(g255_3);
//	//		bH = ceilINT(b255_3);
//
//	//		rL = floorINT(r255_3);
//	//		gL = floorINT(g255_3);
//	//		bL = floorINT(b255_3);
//
//	//		indexH = cubeIndex(rH, gH, bH);
//	//		indexL = cubeIndex(rL, gL, bL);
//
//	//		toR = mixvalue(LUTD3[indexL * 3], LUTD3[indexH * 3], r255_3);
//	//		toG = mixvalue(LUTD3[indexL * 3 + 1], LUTD3[indexH * 3 + 1], g255_3);
//	//		toB = mixvalue(LUTD3[indexL * 3 + 2], LUTD3[indexH * 3 + 2], b255_3);
//
//	//		NewIM.at<Vec3b>(i, j)[2] = uchar(toR / 11);
//	//		NewIM.at<Vec3b>(i, j)[1] = uchar(toG / 11);
//	//		NewIM.at<Vec3b>(i, j)[0] = uchar(toB / 11);
//	//	}
//	//}
//
//
//
//	namedWindow("处理图像", CV_WINDOW_NORMAL);
//	imshow("处理图像", NewIM);
//	bool isSuccess = cv::imwrite("D:\\work\\3dlut\\output_image_a.jpg", NewIM);
//
//	//delete[] text;
//	delete[] char_w;
//
//
//	waitKey(0);
//	return 0;
//}