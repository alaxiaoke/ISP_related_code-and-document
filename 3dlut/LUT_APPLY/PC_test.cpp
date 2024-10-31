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
//    int y, u, v;
//    int r, g, b;
//    int yIndex, uvIndex;
//    int index;
//
//    uint8_t* yuv; uint8_t* rgb; uint8_t* yuv_out;
//    yuv = new uint8_t[3280 * 3 / 2 * 2448];
//    rgb = new uint8_t[3280 * 3 * 2448];
//    yuv_out = new uint8_t[3280 * 3 / 2 * 2448];
//    cv::Mat rgbMat(height, width, CV_8UC3);
//    for (int i = 0; i < totalSize; i++) {
//        yuv[i] = yuvData[i];
//    }
//
//    CLut3D LutHD;
//    LutHD.lutParam.YUV2RGB_coef[0] = 1.402 * 1024; //int16
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
//    LutHD.YUV420toRGB(yuv, rgb, width, height);
//
//
//    for (int a = 0; a < height; a++) {
//        for (int i = 0; i < width; i++) {
//
//            rgbMat.at<cv::Vec3b>(a, i)[0] = rgb[a * width * 3 + i * 3 + 2];
//            rgbMat.at<cv::Vec3b>(a, i)[1] = rgb[a * width * 3 + i * 3 + 1];
//            rgbMat.at<cv::Vec3b>(a, i)[2] = rgb[a * width * 3 + i * 3];
//        }
//    }
//
//    LutHD.RGBtoYUV420(rgb, yuv_out, width, height);
//
//    for (int i = 0; i < totalSize; i++) {
//        yuvData[i] = yuv_out[i];
//    }
//
//    namedWindow("处理图像", CV_WINDOW_NORMAL);
//    imshow("处理图像", rgbMat);
//    waitKey(0);
//
//    // 创建 YUV420P 格式的 Mat 对象
//    cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1, yuvData.data());
//
//    // 创建 RGB 格式的 Mat 对象
//    cv::Mat yuv2rgbMat(height, width, CV_8UC3);
//
//    // 转换 YUV420P 到 RGB
//    cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2RGB_NV21);
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
//    std::ofstream outFile("D:\\work\\3dlut\\yuv_input.txt", std::ios::out | std::ios::binary);
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
//    CLut3D LutHD;
//
//    LutHD.lutParam.RegEnable = 1;
//    LutHD.lutParam.RegImgHeight = 3280;
//    LutHD.lutParam.RegImgWidth = 2448;
//    LutHD.lutParam.YUV2RGB_coef[0] = 1.402 * 4096;
//    LutHD.lutParam.YUV2RGB_coef[1] = -0.344136 * 4096;
//    LutHD.lutParam.YUV2RGB_coef[2] = -0.714136 * 4096;
//    LutHD.lutParam.YUV2RGB_coef[3] = 1.772 * 4096;
//    LutHD.lutParam.YUV2RGB_coef[4] = 128;
//
//    LutHD.lutParam.RGB2YUV_coef[0] = 0.299 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[1] = 0.587 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[2] = 0.114 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[3] = -0.1687 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[4] = -0.3313 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[5] = 0.5 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[6] = 0.5 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[7] = -0.4187 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[8] = -0.0813 * 4096;
//    LutHD.lutParam.RGB2YUV_coef[9] = 128;
//
//    uint8_t* yuv; uint8_t* rgb; uint8_t* rgbout;  uint8_t* yuvout;
//    yuv = new uint8_t[3280 * 3 / 2 * 2448];
//    yuvout = new uint8_t[3280 * 3 / 2 * 2448];
//    rgb = new uint8_t[3280 * 3 * 2448];
//    rgbout = new uint8_t[3280 * 3 * 2448];
//
//    cv::Mat image = cv::imread("D:\\work\\3dlut\\a.bak.jpg");
//    if (image.empty()) {
//        std::cerr << "无法读取图像" << std::endl;
//        return -1;
//    }
//    cv::Size size = image.size();
//    int width = size.width;
//    int height = size.height;
//
//    for (int a = 0; a < height; a++) {
//        for (int i = 0; i < width; i++) {
//
//            rgb[a * width * 3 + i * 3 + 2] = image.at<cv::Vec3b>(a, i)[2];
//            rgb[a * width * 3 + i * 3 + 1] = image.at<cv::Vec3b>(a, i)[1];
//            rgb[a * width * 3 + i * 3] = image.at<cv::Vec3b>(a, i)[0];
//        }
//    }
//    //namedWindow("初始图像", CV_WINDOW_NORMAL);
//    //imshow("初始图像", image);
//    //waitKey(0);
//
//    uint8_t LUTD3[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
//    const char* cube1Path = "D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\bInt.cube";
//    readLUT(LUTD3, cube1Path);
//
//
//    LutHD.ProcessImagePCTest(rgb, LUTD3, rgbout);
//
//    for (int a = 0; a < height; a++) {
//        for (int i = 0; i < width; i++) {
//
//            image.at<cv::Vec3b>(a, i)[2] = rgbout[a * width * 3 + i * 3 + 2];
//            image.at<cv::Vec3b>(a, i)[1] = rgbout[a * width * 3 + i * 3 + 1];
//            image.at<cv::Vec3b>(a, i)[0] = rgbout[a * width * 3 + i * 3];
//        }
//    }
//
//    imshow("RGB", image);
//    waitKey(0);
//
//
//    LutHD.RGBtoYUV420(rgbout, yuvout, width, height);
//
//    int ySize = width * height;
//    int uvSize = ySize / 4;
//    int totalSize = ySize + 2 * uvSize;
//
//    std::vector<uint8_t> yuvData(totalSize);
//
//    for (int i = 0; i < totalSize; i++) {
//        yuvData[i] = yuvout[i];
//    }
//
//    // 创建 YUV420P 格式的 Mat 对象
//    cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1, yuvData.data());
//
//    // 创建 RGB 格式的 Mat 对象
//    cv::Mat yuv2rgbMat(height, width, CV_8UC3);
//
//    // 转换 YUV420P 到 RGB
//    cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2RGB_YV12);
//
//    cv::imshow("yuvMat Image", yuv2rgbMat);
//    cv::waitKey(0);
//
//
//    LutHD.YUV420toRGB(yuvout,rgbout, width, height);
//
//    for (int a = 0; a < height; a++) {
//        for (int i = 0; i < width; i++) {
//
//            image.at<cv::Vec3b>(a, i)[2] = rgbout[a * width * 3 + i * 3 + 2];
//            image.at<cv::Vec3b>(a, i)[1] = rgbout[a * width * 3 + i * 3 + 1];
//            image.at<cv::Vec3b>(a, i)[0] = rgbout[a * width * 3 + i * 3];
//        }
//    }
//    imshow("RGB2", image);
//    waitKey(0);
//
//    //bool isSuccess = cv::imwrite("D:\\work\\3dlut\\output_face.jpg", NewIM);
//    //waitKey(0);
//
//    delete[] yuv;           // 释放数组内存
//    yuv = nullptr;
//    delete[] rgb;
//    rgb = nullptr;
//    delete[] rgbout;
//    rgbout = nullptr;
//    delete[] yuvout;
//    yuvout = nullptr;
//    return 0;
//}
//
