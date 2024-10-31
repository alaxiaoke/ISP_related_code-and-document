////
////void CLut3D::YUV420toRGB(uint8_t* yuv, uint8_t* rgb, int width, int height) {
////
////	int y, u, v;
////	int r, g, b;
////	int yIndex, uvIndex;
////	int index;
////
////	for (int j = 0; j < height; j++) {
////		for (int i = 0; i < width; i++) {
////			yIndex = j * width + i;
////			uvIndex = (j / 2) * (width / 2) + (i / 2);
////
////			y = yuv[yIndex];
////			u = yuv[height * width + uvIndex] - 128;
////			v = yuv[height * width + (height * width / 4) + uvIndex] - 128;
////
////			// Convert YUV to RGB
////			r = (int)(y + 1.402 * v);
////			g = (int)(y - 0.344136 * u - 0.714136 * v);
////			b = (int)(y + 1.772 * u);
////
////			// Clamp values to [0, 255]
////			if (r > 255) r = 255;
////			if (r < 0) r = 0;
////			if (g > 255) g = 255;
////			if (g < 0) g = 0;
////			if (b > 255) b = 255;
////			if (b < 0) b = 0;
////
////			// Store the RGB values
////			index = yIndex * 3;
////			rgb[index] = r;
////			rgb[index + 1] = g;
////			rgb[index + 2] = b;
////		}
////	}
////}
////
////// RGB to YUV conversion function 假设输入的RGB数据是连续存储的，每个像素占用3个字节（R、G、B）
//////RGB转YUV系数：
//////Y = c0 * R + c1 * G + c2 * B
//////U = c3 * R + c4 * G + c5 * B + 128
//////V = c6 * R + c7 * G + c8 * B + 128
////void CLut3D::RGBtoYUV420(const uint8_t* rgb, uint8_t* yuv, int width, int height) {
////
////	const int frameSize = width * height;
////	const int chromaSize = frameSize / 4;
////
////	for (int i = 0, yIdx = 0, uIdx = frameSize, vIdx = frameSize + chromaSize; i < height; ++i) {
////		for (int j = 0; j < width; j += 2) {
////			// Read RGB values
////			uint8_t r1 = rgb[(i * width + j) * 3];
////			uint8_t g1 = rgb[(i * width + j) * 3 + 1];
////			uint8_t b1 = rgb[(i * width + j) * 3 + 2];
////
////			uint8_t r2 = rgb[(i * width + j + 1) * 3];
////			uint8_t g2 = rgb[(i * width + j + 1) * 3 + 1];
////			uint8_t b2 = rgb[(i * width + j + 1) * 3 + 2];
////
////			// Convert to YUV
////			yuv[yIdx++] = (uint8_t)(0.299 * r1 + 0.587 * g1 + 0.114 * b1);
////			yuv[yIdx++] = (uint8_t)(0.299 * r2 + 0.587 * g2 + 0.114 * b2);
////
////			if (i % 2 == 0) { // Only calculate U and V for every other row
////				yuv[uIdx] = (uint8_t)((-0.1687 * r1 - 0.3313 * g1 + 0.5 * b1 + (-0.1687 * r2 - 0.3313 * g2 + 0.5 * b2)) / 2 + 128);
////				yuv[vIdx] = (uint8_t)((0.5 * r1 - 0.4187 * g1 - 0.0813 * b1 + (0.5 * r2 - 0.4187 * g2 - 0.0813 * b2)) / 2 + 128);
////				uIdx++;
////				vIdx++;
////			}
////		}
////	}
////}
//
////lutParam.LUT1 = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.LUT2 = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.LUT3 = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.LUT4 = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.LUT5 = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.LUTcurrent = new uint8_t[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3];
////lutParam.CCTLevel[0] = 2500;
////lutParam.CCTLevel[1] = 4500;
////lutParam.CCTLevel[2] = 6500;
////lutParam.CCTLevel[3] = 8500;
//// 
////int readYUVFile(const std::string& filename, int width, int height) {
////    int ySize = width * height;
////    int uvSize = ySize / 4;
////    int totalSize = ySize + 2 * uvSize;
////
////    // 读取 YUV420 数据
////    std::ifstream file(filename, std::ios::binary);
////    if (!file.is_open()) {
////        std::cerr << "无法打开文件: " << filename << std::endl;
////        return -1;
////    }
////
////    std::vector<uint8_t> yuvData(totalSize);
////    file.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
////    file.close();
//
//    //// 创建 YUV420P 格式的 Mat 对象
//    //cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1, yuvData.data());
//
//    //// 创建 RGB 格式的 Mat 对象
//    //cv::Mat rgbMat(height, width, CV_8UC3);
//
//    //// 转换 YUV420P 到 RGB
//    //cv::cvtColor(yuvMat, rgbMat, cv::COLOR_YUV2RGB_NV21);
//
//    //cv::imshow("rgbMat Image", rgbMat);
//    //cv::waitKey(0);
//
//    //return 0;
////}
//
//
//
////#include "3DLUT_interpolation.h"
////
////#include <opencv2/opencv.hpp>
////#include <opencv2/core/core.hpp>
////#include <opencv2/highgui/highgui.hpp>
////#include <opencv2/highgui/highgui_c.h>
////#include <vector>
////#include <fstream> 
////
////using namespace cv;
////
////int writeImageTxt() {
////    // 读取图像
////    cv::Mat image = cv::imread("D:\\work\\3dlut\\a.bak.jpg");
////    if (image.empty()) {
////        std::cerr << "无法读取图像" << std::endl;
////        return -1;
////    }
////
////    // 打开输出文件
////    std::ofstream outFile("D:\\work\\3dlut\\a_output.txt", std::ios::out | std::ios::binary);
////    if (!outFile.is_open()) {
////        std::cerr << "无法打开输出文件" << std::endl;
////        return -1;
////    }
////
////    // 写入图像数据
////    for (int i = 0; i < image.rows; ++i) {
////        for (int j = 0; j < image.cols; ++j) {
////            for (int k = 0; k < image.channels(); ++k) {
////                unsigned char value = image.at<cv::Vec3b>(i, j)[k];
////                outFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
////            }
////        }
////    }
////
////    // 关闭文件
////    outFile.close();
////    std::cout << "图像数据已成功写入 output.txt" << std::endl;
////
////    return 0;
////}
////
////cv::Mat readImageTxt(const std::string& filename, int rows, int cols) {
////    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
////    if (!inFile.is_open()) {
////        std::cerr << "无法打开输入文件" << std::endl;
////        return cv::Mat();
////    }
////
////    // 创建目标图像
////    cv::Mat image(rows, cols, CV_8UC3);
////
////    // 读取图像数据
////    for (int i = 0; i < rows; ++i) {
////        for (int j = 0; j < cols; ++j) {
////            for (int k = 0; k < 3; ++k) {
////                unsigned char value;
////                inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
////                image.at<cv::Vec3b>(i, j)[k] = value;
////            }
////        }
////    }
////
////    inFile.close();
////    return image;
////}
////
////int showIMG() {
////    cv::Mat image = cv::imread("D:\\work\\3dlut\\a.bak.jpg");
////    if (image.empty()) {
////        std::cerr << "无法读取图像" << std::endl;
////        return -1;
////    }
////    if (image.channels() != 3) {
////        printf("The image is not a 3-channel image\n");
////        return -1;
////    }
////
////    cv::Mat imageFromTxt = readImageTxt("D:\\work\\3dlut\\a_output.txt", image.rows, image.cols);
////    if (imageFromTxt.empty()) {
////        std::cerr << "无法加载图像数据" << std::endl;
////        return -1;
////    }
////
////    // 显示图像
////    cv::imshow("Loaded Image", imageFromTxt);
////    cv::waitKey(0);
////}
////
////void readLUT(uint8_t* LUTD3, const char* cubePath) {
////    //step1 读取*.cube文件
////
////    char* text, * point_ext;
////    FILE* pf = fopen(cubePath, "rb+");
////    if (pf == NULL)
////    {
////        printf("error!");
////        return;
////    }
////
////    fseek(pf, 0, SEEK_END);
////    long lSize = ftell(pf);
////
////    text = new char[lSize];
////    rewind(pf);
////
////    fread(text, sizeof(char), lSize, pf);
////    char* char_w = new char[8];
////
////    int t(0), i(0), itN(0), invat(0);
////
////    //定位到有效数据的起始位置
////    point_ext = text;
////
////    for (i = 8; i <= lSize; i += 8)
////    {
////        memcpy(char_w, point_ext + i - 8, 8);
////        LUTD3[t++] = atoi(char_w);
////        //printf("%.6d\n", LUTD3[t-1]);
////        memset(char_w, 0, 8);
////
////    }
////    delete[] char_w;
////}
////
////int main() {
////
////    int rows = 0;
////    int cols = 0;
////    // 创建一个数组来存储图像数据
////    unsigned char* rgb_buff; unsigned char* rgb_out_buff;
////    rgb_buff = new unsigned char[3 * 4096 * 4096];
////    rgb_out_buff = new unsigned char[3 * 4096 * 4096];
////    // 读取图像数据
////    std::ifstream inFile("D:\\work\\3dlut\\a_output.txt", std::ios::in | std::ios::binary);
////    if (!inFile.is_open()) {
////        std::cerr << "无法打开输入文件" << std::endl;
////        return -1;
////    }
////    for (int i = 0; i < rows; ++i) {
////        for (int j = 0; j < cols; ++j) {
////            for (int k = 0; k < 3; ++k) {
////                unsigned char value;
////                inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
////                rgb_buff = value;
////            }
////        }
////    }
////
////
////
////    //// 将图像数据复制到数组中
////    //for (int i = 0; i < rows; ++i) {
////    //    for (int j = 0; j < cols; ++j) {
////    //        rgb_buff[i * image.cols * 3 + j * 3] = image.at<cv::Vec3b>(i, j)[0];
////    //        rgb_buff[i * image.cols * 3 + j * 3 + 1] = image.at<cv::Vec3b>(i, j)[1];
////    //        rgb_buff[i * image.cols * 3 + j * 3 + 2] = image.at<cv::Vec3b>(i, j)[2];
////    //    }
////    //}
////    //// 打印数组中的前几个元素
////    //for (int i = 0; i < 100; i+=3) {
////    //    std::cout << "Pixel " << i << ": B=" << (int)rgb_buff[i]
////    //        << ", G=" << (int)rgb_buff[i+1]
////    //        << ", R=" << (int)rgb_buff[i+2] << std::endl;
////    //}
////
////    // 初始化LUT
////    CLut3D LutHD;
////    uint8_t LUTD3[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
////
////
////    // 初始化参数
////    LutHD.lutParam.RegEnable = 1;
////    LutHD.lutParam.RegImgWidth = image.rows;
////    LutHD.lutParam.RegImgHeight = image.cols;
////    const char* cube1Path = "D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\dInt.cube";
////    readLUT(LUTD3, cube1Path);
////
////    // LUT作用
////    LutHD.ProcessImage(rgb_buff, LUTD3, rgb_out_buff);
////
////    namedWindow("原图像", CV_WINDOW_NORMAL);
////    imshow("原图像", image);
////
////    Mat NewIM = image.clone();
////    for (int i = 0; i < image.rows; ++i) {
////        for (int j = 0; j < image.cols; ++j) {
////            NewIM.at<cv::Vec3b>(i, j)[0] = rgb_out_buff[i * image.cols * 3 + j * 3];
////            NewIM.at<cv::Vec3b>(i, j)[1] = rgb_out_buff[i * image.cols * 3 + j * 3 + 1];
////            NewIM.at<cv::Vec3b>(i, j)[2] = rgb_out_buff[i * image.cols * 3 + j * 3 + 2];
////        }
////    }
////    namedWindow("处理图像", CV_WINDOW_NORMAL);
////    imshow("处理图像", NewIM);
////    bool isSuccess = cv::imwrite("D:\\work\\3dlut\\output_face.jpg", NewIM);
////    waitKey(0);
////
////    delete[] rgb_buff;           // 释放数组内存
////    rgb_buff = nullptr;
////    delete[] rgb_out_buff;           // 释放数组内存
////    rgb_out_buff = nullptr;
////
////    return 0;
////}
////
//
////int main()
////{
////	//step1 读取*.cube文件
////
////	char* text, * point_ext;
////	FILE* pf = fopen("D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\faceInt.cube", "rb+");
////	if (pf == NULL)
////	{
////		printf("error!");
////		return -1;
////	}
////
////	fseek(pf, 0, SEEK_END);
////	long lSize = ftell(pf);
////
////	text = new char[lSize];
////	rewind(pf);
////
////	fread(text, sizeof(char), lSize, pf);
////	char* char_w = new char[8];
////	int LUTD3[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
////	//int LUTD3_1[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
////
////	int t(0), i(0), itN(0), invat(0);
////
////	//定位到有效数据的起始位置
////	point_ext = text;
////
////	for (i = 8; i <= lSize; i += 8)
////	{
////		memcpy(char_w, point_ext + i - 8, 8);
////		LUTD3[t++] = atoi(char_w);
////		//LUTD3_1[t] = 12;
////		//printf("%.6d\n", LUTD3[t-1]);
////		memset(char_w, 0, 8);
////
////	}
////
////	// step2 读取图像
////
////	Mat ORI = imread("D:\\work\\3dlut\\11.jpg");  //1234.bmp/cat.jpg/detailIM.jpg
////	namedWindow("原图像", CV_WINDOW_NORMAL);
////	imshow("原图像", ORI);
////
////	// step3 3D_LUT arith
////
////	Mat ORI_normfloat = Mat(ORI.size(), CV_32FC3);
////	Mat NewIM = ORI.clone();
////
////	unsigned char rH, gH, bH, rL, gL, bL;
////	int index000, index001, index010, index011, index100, index101, index110, index111;
////	uint32_t c1 = 0, c2 = 0, c3 = 0;
////	uint32_t rout = 0, gout = 0, bout = 0;
////
////	uint32_t frgb = 0;
////	uint32_t len = LUT_SIZE;
////	uint32_t r_rem = 0, g_rem = 0, b_rem = 0;
////
////	for (int i = 0; i < ORI.rows; i++)
////	{
////		for (int j = 0; j < ORI.cols; j++)
////		{
////			int r255 = (ORI.at<Vec3b>(i, j)[2]);
////			int g255 = (ORI.at<Vec3b>(i, j)[1]);
////			int b255 = (ORI.at<Vec3b>(i, j)[0]);
////
////			//r255 = 128;
////			//g255 = 128;
////			//b255 = 128;
////			//printf("r255:%.6d\n", r255);
////			int r255_2 = r255 << SHIFT;
////			int g255_2 = g255 << SHIFT;
////			int b255_2 = b255 << SHIFT;
////			//printf("r255_2:%.6d\n", r255_2);
////			int r255_3 = r255_2 / 255 * (LUT_SIZE - 1);			//直接右移的方法得到ceil和floor
////			int g255_3 = g255_2 / 255 * (LUT_SIZE - 1);
////			int b255_3 = b255_2 / 255 * (LUT_SIZE - 1);
////			//printf("r255_3:%.6d\n", r255_3);
////
////			rH = ceilINT(r255_3);
////			gH = ceilINT(g255_3);
////			bH = ceilINT(b255_3);
////
////			rL = floorINT(r255_3);
////			gL = floorINT(g255_3);
////			bL = floorINT(b255_3);
////
////			index000 = cubeIndex(rL, gL, bL);
////			index001 = cubeIndex(rL, gL, bH);
////			index010 = cubeIndex(rL, gH, bL);
////			index011 = cubeIndex(rL, gH, bH);
////			index100 = cubeIndex(rH, gL, bL);
////			index101 = cubeIndex(rH, gL, bH);
////			index110 = cubeIndex(rH, gH, bL);
////			index111 = cubeIndex(rH, gH, bH);
////
////
////			ColorTriplet<int> c000 = { LUTD3[index000 * 3], LUTD3[index000 * 3 + 1], LUTD3[index000 * 3 + 2] };
////			ColorTriplet<int> c001 = { LUTD3[index001 * 3], LUTD3[index001 * 3 + 1], LUTD3[index001 * 3 + 2] };
////			ColorTriplet<int> c010 = { LUTD3[index010 * 3], LUTD3[index010 * 3 + 1], LUTD3[index010 * 3 + 2] };
////			ColorTriplet<int> c011 = { LUTD3[index011 * 3], LUTD3[index011 * 3 + 1], LUTD3[index011 * 3 + 2] };
////			ColorTriplet<int> c100 = { LUTD3[index100 * 3], LUTD3[index100 * 3 + 1], LUTD3[index100 * 3 + 2] };
////			ColorTriplet<int> c101 = { LUTD3[index101 * 3], LUTD3[index101 * 3 + 1], LUTD3[index101 * 3 + 2] };
////			ColorTriplet<int> c110 = { LUTD3[index110 * 3], LUTD3[index110 * 3 + 1], LUTD3[index110 * 3 + 2] };
////			ColorTriplet<int> c111 = { LUTD3[index111 * 3], LUTD3[index111 * 3 + 1], LUTD3[index111 * 3 + 2] };
////
////			//int tmp2 = pow(2, SHIFT);
////			//int r_rem1 = r255_3 % tmp2;
////
////			r_rem = r255_3 & ((1 << SHIFT) - 1);
////			g_rem = g255_3 & ((1 << SHIFT) - 1);
////			b_rem = b255_3 & ((1 << SHIFT) - 1);
////
////			//r_rem = 0;
////			//g_rem = 0;
////			//b_rem = 0;
////
////			uint32_t casev = 0;
////			if (r_rem > g_rem && g_rem >= b_rem) casev = 1;
////			else if (r_rem >= b_rem && b_rem > g_rem) casev = 2;
////			else if (b_rem >= r_rem && r_rem >= g_rem) casev = 3;
////			else if (g_rem >= r_rem && r_rem > b_rem) casev = 4;
////			else if (g_rem > b_rem && b_rem >= r_rem) casev = 5;
////			else casev = 6;
////
//
//
//
////switch (casev)
////{
////case 1:
////	rout = c000.R + ((((c100.R - c000.R) * r_rem + (c110.R - c100.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c100.G - c000.G) * r_rem + (c110.G - c100.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c100.B - c000.B) * r_rem + (c110.B - c100.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////case 2:
////	rout = c000.R + ((((c100.R - c000.R) * r_rem + (c111.R - c101.R) * g_rem + (c101.R - c100.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c100.G - c000.G) * r_rem + (c111.G - c101.G) * g_rem + (c101.G - c100.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c100.B - c000.B) * r_rem + (c111.B - c101.B) * g_rem + (c101.B - c100.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////case 3:
////	rout = c000.R + ((((c101.R - c001.R) * r_rem + (c111.R - c101.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c101.G - c001.G) * r_rem + (c111.G - c101.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c101.B - c001.B) * r_rem + (c111.B - c101.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////case 4:
////	rout = c000.R + ((((c110.R - c010.R) * r_rem + (c010.R - c000.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c110.G - c010.G) * r_rem + (c010.G - c000.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c110.B - c010.B) * r_rem + (c010.B - c000.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////case 5:
////	rout = c000.R + ((((c111.R - c011.R) * r_rem + (c010.R - c000.R) * g_rem + (c011.R - c010.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c111.G - c011.G) * r_rem + (c010.G - c000.G) * g_rem + (c011.G - c010.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c111.B - c011.B) * r_rem + (c010.B - c000.B) * g_rem + (c011.B - c010.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////case 6:
////	rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	gout = c000.G + ((((c111.G - c011.G) * r_rem + (c011.G - c001.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////	bout = c000.B + ((((c111.B - c011.B) * r_rem + (c011.B - c001.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1) + 1)) >> SHIFT);
////
////	break;
////default:
////	break;
////}
////			//bout = bout < 0 ? 0 : bout;
////			//gout = gout < 0 ? 0 : gout;
////			//rout = rout < 0 ? 0 : rout;
////
////			//bout = bout > 255 ? 255 : bout;
////			//gout = gout > 255 ? 255 : gout;
////			//rout = rout > 255 ? 255 : rout;
////
////			NewIM.at<Vec3b>(i, j)[0] = uchar(bout);
////			NewIM.at<Vec3b>(i, j)[1] = uchar(gout);
////			NewIM.at<Vec3b>(i, j)[2] = uchar(rout);
////		}
////
////	}
////	
////
////
////	namedWindow("处理图像", CV_WINDOW_NORMAL);
////	imshow("处理图像", NewIM);
////	bool isSuccess = cv::imwrite("D:\\work\\3dlut\\output_face.jpg", NewIM);
////
////	//delete[] text;
////	delete[] char_w;
////
////
////	waitKey(0);
////	return 0;
////}
//
//
//
//
////int CLut3D::cubeMerge(int cctNow) {
////
////	if (cctNow < 0 || cctNow > 13000){
////		return -1;
////	}
////	int LUTNodeCount = LUT_SIZE * LUT_SIZE * LUT_SIZE * 3;
////	int LutMergeValue = 0;
////	int weight1, weight2;
////	if (cctNow <= lutParam.CCTLevel[0] - 200) {
////		memcpy(lutParam.LUTcurrent, lutParam.LUT1, LUTNodeCount * sizeof(uint8_t));
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[0] + 200) {
////		weight2 = ((cctNow - (lutParam.CCTLevel[0] - 200)) << 8) / 400;
////		weight1 = (1 << 8) - weight2;
////		for (int i = 0; i < LUTNodeCount; i++) {
////			lutParam.LUTcurrent[i] = (lutParam.LUT1[i] * weight1 + lutParam.LUT2[i] * weight2) >> 8;
////		}
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[1] - 200) {
////		memcpy(lutParam.LUTcurrent, lutParam.LUT2, LUTNodeCount * sizeof(uint8_t));
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[1] + 200) {
////		weight2 = ((cctNow - (lutParam.CCTLevel[1] - 200)) << 8) / 400;
////		weight1 = (1 << 8) - weight2;
////		for (int i = 0; i < LUTNodeCount; i++) {
////			lutParam.LUTcurrent[i] = (lutParam.LUT2[i] * weight1 + lutParam.LUT3[i] * weight2) >> 8;
////		}
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[2] - 200) {
////		memcpy(lutParam.LUTcurrent, lutParam.LUT3, LUTNodeCount * sizeof(uint8_t));
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[2] + 200) {
////		weight2 = ((cctNow - (lutParam.CCTLevel[2] - 200)) << 8) / 400;
////		weight1 = (1 << 8) - weight2;
////		for (int i = 0; i < LUTNodeCount; i++) {
////			lutParam.LUTcurrent[i] = (lutParam.LUT3[i] * weight1 + lutParam.LUT4[i] * weight2) >> 8;
////		}
////		return 0;
////	}
////	else if (cctNow <= lutParam.CCTLevel[3] - 200) {
////		memcpy(lutParam.LUTcurrent, lutParam.LUT4, LUTNodeCount * sizeof(uint8_t));
////		return 0;
////	}
////
////	else if (cctNow <= lutParam.CCTLevel[3] + 200) {
////		weight2 = ((cctNow - (lutParam.CCTLevel[3] - 200)) << 8) / 400;
////		weight1 = (1 << 8) - weight2;
////		for (int i = 0; i < LUTNodeCount; i++) {
////			lutParam.LUTcurrent[i] = (lutParam.LUT4[i] * weight1 + lutParam.LUT5[i] * weight2) >> 8;
////		}
////		return 0;
////	}
////	else {
////		memcpy(lutParam.LUTcurrent, lutParam.LUT5, LUTNodeCount * sizeof(uint8_t));
////		return 0;
////	}
////
////	return 0;
////}
//
//
//
////int CLut3D::ProcessImagePCTest(uint8_t* rgb, uint8_t* LUTD3, uint8_t* rgbout)
////{
////
////	int width = lutParam.RegImgWidth;
////	int height = lutParam.RegImgHeight;
////
////
////	unsigned char rH, gH, bH, rL, gL, bL;
////	int index000, index001, index010, index011, index100, index101, index110, index111;
////	int32_t c1 = 0, c2 = 0, c3 = 0;
////	int rout = 0, gout = 0, bout = 0;
////
////	uint32_t len = LUT_SIZE;
////	int32_t r_rem = 0, g_rem = 0, b_rem = 0;
////
////	for (int i = 0; i < width; i++)
////	{
////		for (int j = 0; j < height; j++)
////		{
////			int b = (rgb[i * height * 3 + j * 3]);
////			int g = (rgb[i * height * 3 + j * 3 + 1]);
////			int r = (rgb[i * height * 3 + j * 3 + 2]);
////
////			int r255 = r << SHIFT;
////			int g255 = g << SHIFT;
////			int b255 = b << SHIFT;
////			// 
////			int r255_2 = (r255 >> 8) * (LUT_SIZE - 1);			//直接右移的方法得到ceil和floor
////			int g255_2 = (g255 >> 8) * (LUT_SIZE - 1);
////			int b255_2 = (b255 >> 8) * (LUT_SIZE - 1);
////
////			//printf("r255_3:%.6d\n", r255_3);
////
////			rL = r255_2 >> SHIFT;
////			gL = g255_2 >> SHIFT;
////			bL = b255_2 >> SHIFT;
////
////			rH = rL + 1;
////			gH = gL + 1;
////			bH = bL + 1;
////
////			index000 = cubeIndex(rL, gL, bL);
////			index001 = cubeIndex(rL, gL, bH);
////			index010 = cubeIndex(rL, gH, bL);
////			index011 = cubeIndex(rL, gH, bH);
////			index100 = cubeIndex(rH, gL, bL);
////			index101 = cubeIndex(rH, gL, bH);
////			index110 = cubeIndex(rH, gH, bL);
////			index111 = cubeIndex(rH, gH, bH);
////
////			ColorTriplet c000 = { LUTD3[index000 * 3], LUTD3[index000 * 3 + 1], LUTD3[index000 * 3 + 2] };
////			ColorTriplet c001 = { LUTD3[index001 * 3], LUTD3[index001 * 3 + 1], LUTD3[index001 * 3 + 2] };
////			ColorTriplet c010 = { LUTD3[index010 * 3], LUTD3[index010 * 3 + 1], LUTD3[index010 * 3 + 2] };
////			ColorTriplet c011 = { LUTD3[index011 * 3], LUTD3[index011 * 3 + 1], LUTD3[index011 * 3 + 2] };
////			ColorTriplet c100 = { LUTD3[index100 * 3], LUTD3[index100 * 3 + 1], LUTD3[index100 * 3 + 2] };
////			ColorTriplet c101 = { LUTD3[index101 * 3], LUTD3[index101 * 3 + 1], LUTD3[index101 * 3 + 2] };
////			ColorTriplet c110 = { LUTD3[index110 * 3], LUTD3[index110 * 3 + 1], LUTD3[index110 * 3 + 2] };
////			ColorTriplet c111 = { LUTD3[index111 * 3], LUTD3[index111 * 3 + 1], LUTD3[index111 * 3 + 2] };
////
////			r_rem = r255_2 & ((1 << SHIFT) - 1);
////			g_rem = g255_2 & ((1 << SHIFT) - 1);
////			b_rem = b255_2 & ((1 << SHIFT) - 1);
////
////			uint32_t casev = 0;
////			if (r_rem > g_rem && g_rem >= b_rem) casev = 1;
////			else if (r_rem >= b_rem && b_rem > g_rem) casev = 2;
////			else if (b_rem >= r_rem && r_rem >= g_rem) casev = 3;
////			else if (g_rem >= r_rem && r_rem > b_rem) casev = 4;
////			else if (g_rem > b_rem && b_rem >= r_rem) casev = 5;
////			else casev = 6;
////
////			switch (casev)
////			{
////			case 1:
////				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c110.R - c100.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c110.G - c100.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c110.B - c100.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			case 2:
////				rout = c000.R + ((((c100.R - c000.R) * r_rem + (c111.R - c101.R) * g_rem + (c101.R - c100.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c100.G - c000.G) * r_rem + (c111.G - c101.G) * g_rem + (c101.G - c100.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c100.B - c000.B) * r_rem + (c111.B - c101.B) * g_rem + (c101.B - c100.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			case 3:
////				rout = c000.R + ((((c101.R - c001.R) * r_rem + (c111.R - c101.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c101.G - c001.G) * r_rem + (c111.G - c101.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c101.B - c001.B) * r_rem + (c111.B - c101.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			case 4:
////				rout = c000.R + ((((c110.R - c010.R) * r_rem + (c010.R - c000.R) * g_rem + (c111.R - c110.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c110.G - c010.G) * r_rem + (c010.G - c000.G) * g_rem + (c111.G - c110.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c110.B - c010.B) * r_rem + (c010.B - c000.B) * g_rem + (c111.B - c110.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			case 5:
////				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c010.G - c000.G) * g_rem + (c011.G - c010.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c010.B - c000.B) * g_rem + (c011.B - c010.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			case 6:
////				rout = c000.R + ((((c111.R - c011.R) * r_rem + (c011.R - c001.R) * g_rem + (c001.R - c000.R) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				gout = c000.G + ((((c111.G - c011.G) * r_rem + (c011.G - c001.G) * g_rem + (c001.G - c000.G) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////				bout = c000.B + ((((c111.B - c011.B) * r_rem + (c011.B - c001.B) * g_rem + (c001.B - c000.B) * b_rem) + (1 << (SHIFT - 1))) >> SHIFT);
////
////				break;
////			default:
////				break;
////			}
////
////			bout = bout < 0 ? 0 : bout;
////			gout = gout < 0 ? 0 : gout;
////			rout = rout < 0 ? 0 : rout;
////
////			bout = bout > 255 ? 255 : bout;
////			gout = gout > 255 ? 255 : gout;
////			rout = rout > 255 ? 255 : rout;
////
////			rgbout[i * height * 3 + j * 3] = bout;
////			rgbout[i * height * 3 + j * 3 + 1] = gout;
////			rgbout[i * height * 3 + j * 3 + 2] = rout;
////
////		}
////	}
////
////	return 0;
////}
//
//#include "3DLUT_interpolation.h"
//
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/highgui/highgui_c.h>
//#include <vector>
//#include <fstream> 
//
//using namespace cv;
//
//int writeImageTxt() {
//    // 读取图像
//    cv::Mat image = cv::imread("D:\\work\\3dlut\\a.bak.jpg");
//    if (image.empty()) {
//        std::cerr << "无法读取图像" << std::endl;
//        return -1;
//    }
//
//    // 打开输出文件
//    std::ofstream outFile("D:\\work\\3dlut\\a_output.txt", std::ios::out | std::ios::binary);
//    if (!outFile.is_open()) {
//        std::cerr << "无法打开输出文件" << std::endl;
//        return -1;
//    }
//
//    // 写入图像数据
//    for (int i = 0; i < image.rows; ++i) {
//        for (int j = 0; j < image.cols; ++j) {
//            for (int k = 0; k < image.channels(); ++k) {
//                unsigned char value = image.at<cv::Vec3b>(i, j)[k];
//                outFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
//            }
//        }
//    }
//
//    // 关闭文件
//    outFile.close();
//    std::cout << "图像数据已成功写入 output.txt" << std::endl;
//
//    return 0;
//}
//
//int readYUVFile(const std::string& filename, int width, int height) {
//    int ySize = width * height;
//    int uvSize = ySize / 4;
//    int totalSize = ySize + 2 * uvSize;
//
//    // 读取 YUV420 数据
//    std::ifstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::cerr << "无法打开文件: " << filename << std::endl;
//        return -1;
//    }
//
//    std::vector<uint8_t> yuvData(totalSize);
//    file.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
//    file.close();
//
//
//    // 创建 YUV420P 格式的 Mat 对象
//    cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1, yuvData.data());
//
//    // 创建 RGB 格式的 Mat 对象
//    cv::Mat yuv2rgbMat(height, width, CV_8UC3);
//
//    // 转换 YUV420P 到 RGB
//    cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2BGR_YV12);
//
//    cv::imshow("rgbMat Image", yuv2rgbMat);
//    cv::waitKey(0);
//
//}
//
//void writeYUVTxt(const std::string& filename, int width, int height) {
//    int ySize = width * height;
//    int uvSize = ySize / 4;
//    int totalSize = ySize + 2 * uvSize;
//
//    // 读取 YUV420 数据
//    std::ifstream file(filename, std::ios::binary);
//    if (!file.is_open()) {
//        std::cerr << "无法打开文件: " << filename << std::endl;
//        return;
//    }
//
//    std::vector<uint8_t> yuvData(totalSize);
//    file.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
//    file.close();
//
//    // 打开输出文件
//    std::ofstream outFile("D:\\work\\3dlut\\yuv_input_3.txt", std::ios::out | std::ios::binary);
//    if (!outFile.is_open()) {
//        std::cerr << "无法打开输出文件" << std::endl;
//        return;
//    }
//
//    // 写入图像数据
//    for (int i = 0; i < totalSize; ++i) {
//
//        outFile.write(reinterpret_cast<const char*>(&yuvData[i]), sizeof(uint8_t));
//    }
//
//    // 关闭文件
//    outFile.close();
//    std::cout << "图像数据已成功写入 yuv_input.txt" << std::endl;
//
//
//    return;
//}
//
//void readYUVTxt(const std::string& filename, int width, int height, uint8_t* yuv) {
//    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
//    if (!inFile.is_open()) {
//        std::cerr << "无法打开输入文件" << std::endl;
//        return;
//    }
//    int ySize = width * height;
//    int uvSize = ySize / 4;
//    int totalSize = ySize + 2 * uvSize;
//
//    std::vector<uint8_t> yuvData(totalSize);
//    inFile.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
//    inFile.close();
//
//    for (int i = 0; i < totalSize; i++) {
//        yuv[i] = yuvData[i];
//    }
//
//    //// 创建 YUV420P 格式的 Mat 对象
//    //cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1, yuvData.data());
//
//    //// 创建 RGB 格式的 Mat 对象
//    //cv::Mat yuv2rgbMat(height, width, CV_8UC3);
//
//    //// 转换 YUV420P 到 RGB
//    //cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2RGB_NV21);
//
//    //cv::imshow("rgbMat Image", yuv2rgbMat);
//    //cv::waitKey(0);
//
//    return;
//}
//
//cv::Mat readImageTxt(const std::string& filename, int rows, int cols) {
//    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
//    if (!inFile.is_open()) {
//        std::cerr << "无法打开输入文件" << std::endl;
//        return cv::Mat();
//    }
//
//    // 创建目标图像
//    cv::Mat image(rows, cols, CV_8UC3);
//
//    // 读取图像数据
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < cols; ++j) {
//            for (int k = 0; k < 3; ++k) {
//                unsigned char value;
//                inFile.read(reinterpret_cast<char*>(&value), sizeof(value));
//                image.at<cv::Vec3b>(i, j)[k] = value;
//            }
//        }
//    }
//
//    inFile.close();
//    return image;
//}
//
//int showIMG() {
//    cv::Mat image = cv::imread("D:\\work\\3dlut\\a.bak.jpg");
//    if (image.empty()) {
//        std::cerr << "无法读取图像" << std::endl;
//        return -1;
//    }
//    if (image.channels() != 3) {
//        printf("The image is not a 3-channel image\n");
//        return -1;
//    }
//
//    cv::Mat imageFromTxt = readImageTxt("D:\\work\\3dlut\\a_output.txt", image.rows, image.cols);
//    if (imageFromTxt.empty()) {
//        std::cerr << "无法加载图像数据" << std::endl;
//        return -1;
//    }
//
//    // 显示图像
//    cv::imshow("Loaded Image", imageFromTxt);
//    cv::waitKey(0);
//}
//
//void readLUT(uint8_t* LUTD3, const char* cubePath) {
//    //step1 读取*.cube文件
//
//    char* text, * point_ext;
//    FILE* pf = fopen(cubePath, "rb+");
//    if (pf == NULL)
//    {
//        printf("error!");
//        return;
//    }
//
//    fseek(pf, 0, SEEK_END);
//    long lSize = ftell(pf);
//
//    text = new char[lSize];
//    rewind(pf);
//
//    fread(text, sizeof(char), lSize, pf);
//    char* char_w = new char[8];
//
//    int t(0), i(0), itN(0), invat(0);
//
//    //定位到有效数据的起始位置
//    point_ext = text;
//
//    for (i = 8; i <= lSize; i += 8)
//    {
//        memcpy(char_w, point_ext + i - 8, 8);
//        LUTD3[t++] = atoi(char_w);
//        //printf("%.6d\n", LUTD3[t-1]);
//        memset(char_w, 0, 8);
//
//    }
//    delete[] char_w;
//}
//
//int main() {
//
//    //writeYUVTxt("D:\\work\\3dlut\\input\\8.yuv.ycc420p", 2448, 1744);
//    //readYUVFile("D:\\work\\3dlut\\input\\133.yuv",2448, 3280);
//
//    CLut3D LutHD;
//
//    LutHD.lutParam.RegEnable = 1;
//    LutHD.lutParam.RegImgHeight = 3280;
//    LutHD.lutParam.RegImgWidth = 2448;
//    LutHD.lutParam.YUV2RGB_coef[0] = 1.402 * 1024;
//    LutHD.lutParam.YUV2RGB_coef[1] = -0.344136 * 1024;
//    LutHD.lutParam.YUV2RGB_coef[2] = -0.714136 * 1024;
//    LutHD.lutParam.YUV2RGB_coef[3] = 1.772 * 1024;
//
//    LutHD.lutParam.RGB2YUV_coef[0] = 0.299 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[1] = 0.587 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[2] = 0.114 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[3] = -0.1687 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[4] = -0.3313 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[5] = 0.5 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[6] = 0.5 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[7] = -0.4187 * 1024;
//    LutHD.lutParam.RGB2YUV_coef[8] = -0.0813 * 1024;
//
//    uint8_t* yuv; uint8_t* yuvout;
//    yuv = new uint8_t[LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight * 3 / 2];
//    yuvout = new uint8_t[LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight * 3 / 2];
//
//    readYUVTxt("D:\\work\\3dlut\\yuv_input.txt", LutHD.lutParam.RegImgWidth, LutHD.lutParam.RegImgHeight, yuv);
//
//    uint8_t LUTD3[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
//    const char* cube1Path = "D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\bInt.cube";
//    readLUT(LUTD3, cube1Path);
//
//    LutHD.ProcessImage(yuv, LUTD3, yuvout);
//
//
//    int ySize = LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight;
//    int uvSize = ySize / 4;
//    int totalSize = ySize + 2 * uvSize;
//
//    std::vector<uint8_t> yuvData(totalSize);
//
//    for (int i = 0; i < totalSize; i++) {
//        yuvData[i] = yuvout[i];
//    }
//
//
//    // 创建 YUV420P 格式的 Mat 对象
//    cv::Mat yuvMat(LutHD.lutParam.RegImgHeight * 3 / 2, LutHD.lutParam.RegImgWidth, CV_8UC1, yuvData.data());
//
//    // 创建 RGB 格式的 Mat 对象
//    cv::Mat yuv2rgbMat(LutHD.lutParam.RegImgHeight, LutHD.lutParam.RegImgWidth, CV_8UC3);
//
//    // 转换 YUV420P 到 RGB
//    cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2RGB_YV12);
//
//    cv::imshow("rgbMat Image", yuv2rgbMat);
//    cv::waitKey(0);
//
//
//    //bool isSuccess = cv::imwrite("D:\\work\\3dlut\\output_face.jpg", NewIM);
//    //waitKey(0);
//
//    delete[] yuv;           // 释放数组内存
//    yuv = nullptr;
//
//    delete[] yuvout;
//    yuvout = nullptr;
//    return 0;
//}
//
