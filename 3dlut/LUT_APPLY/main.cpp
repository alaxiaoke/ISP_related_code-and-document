#include "3DLUT_interpolation.h"

#include <vector>
#include <fstream> 
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>


int readYUVFile(const std::string& filename, int width, int height) {
    int ySize = width * height;
    int uvSize = ySize / 4;
    int totalSize = ySize + 2 * uvSize;

    // 读取 YUV420 数据
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return -1;
    }

    std::vector<uint8_t> yuvData(totalSize);
    file.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
    file.close();

}


void readYUVTxt(const std::string& filename, int width, int height, uint8_t* yuv) {
    std::ifstream inFile(filename, std::ios::in | std::ios::binary);
    if (!inFile.is_open()) {
        return;
    }
    int ySize = width * height;
    int uvSize = ySize / 4;
    int totalSize = ySize + 2 * uvSize;

    std::vector<uint8_t> yuvData(totalSize);
    inFile.read(reinterpret_cast<char*>(yuvData.data()), totalSize);
    inFile.close();

    for (int i = 0; i < totalSize; i++) {
        yuv[i] = yuvData[i];
    }

    return;
}


void readLUT(uint8_t*LUTD3, const char* cubePath) {
    //step1 读取*.cube文件

    char* text, * point_ext;
    FILE* pf = fopen(cubePath, "rb+");
    if (pf == NULL)
    {
        printf("error!");
        return;
    }

    fseek(pf, 0, SEEK_END);
    long lSize = ftell(pf);

    text = new char[lSize];
    rewind(pf);

    fread(text, sizeof(char), lSize, pf);
    char* char_w = new char[8];

    int t(0), i(0), itN(0), invat(0);

    //定位到有效数据的起始位置
    point_ext = text;

    for (i = 8; i <= lSize; i += 8)
    {
        memcpy(char_w, point_ext + i - 8, 8);
        LUTD3[t++] = atoi(char_w);
        //printf("%.6d\n", LUTD3[t-1]);
        memset(char_w, 0, 8);

    }
    delete[] char_w;
}

int main() {

    //1.load接口参数
    CLut3D LutHD;
    //LutHD.SaveSettingFile("D:\\work\\3dlut\\coef_input.txt");
    LutHD.LoadSettingFile("D:\\work\\3dlut\\coef_input.txt");

    //2.load输入YUV420格式的图像二进制文件
    uint8_t* yuv; uint8_t* yuvout;
    yuv = new uint8_t[LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight * 3 / 2];
    yuvout = new uint8_t[LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight * 3 / 2];

    readYUVTxt("D:\\work\\3dlut\\yuv_input.txt", LutHD.lutParam.RegImgWidth, LutHD.lutParam.RegImgHeight, yuv);

    //3.load 3dlut
    uint8_t LUTD3[LUT_SIZE * LUT_SIZE * LUT_SIZE * 3] = { 0 };
    const char* cube1Path = "D:\\work\\3dlut\\3DLUT_IMFILTER-main\\CUBE\\3dlut_lib\\bInt.cube";
    readLUT(LUTD3, cube1Path);

    //4.作用3dlut
    LutHD.ProcessImage(yuv, LUTD3, yuvout);

    //5.opencv展示作用后的图像效果
    int ySize = LutHD.lutParam.RegImgWidth * LutHD.lutParam.RegImgHeight;
    int uvSize = ySize / 4;
    int totalSize = ySize + 2 * uvSize;

    std::vector<uint8_t> yuvData(totalSize);

    for (int i = 0; i < totalSize; i++) {
        yuvData[i] = yuvout[i];
    }
        // 创建 YUV420P 格式的 Mat 对象
        cv::Mat yuvMat(LutHD.lutParam.RegImgHeight * 3 / 2, LutHD.lutParam.RegImgWidth, CV_8UC1, yuvData.data());
    
        // 创建 RGB 格式的 Mat 对象
        cv::Mat yuv2rgbMat(LutHD.lutParam.RegImgHeight, LutHD.lutParam.RegImgWidth, CV_8UC3);
    
        // 转换 YUV420P 到 RGB
        cv::cvtColor(yuvMat, yuv2rgbMat, cv::COLOR_YUV2RGB_YV12);
    
        cv::imshow("rgbMat Image", yuv2rgbMat);
        cv::waitKey(0);

     //6.释放数组内存
    delete[] yuv;           
    yuv = nullptr;
 
    delete[] yuvout;
    yuvout = nullptr;
    return 0;
}

