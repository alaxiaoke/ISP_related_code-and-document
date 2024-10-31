#pragma once
#include<iostream>
using namespace std;

class AWBAlgo
{
public:
	void readStats(int* p, int blknum_);
    void readLightXML(const char *file_name);
    void readWeightXML(const char *file_name);
    void readSeriesXML(const char *file_name);
    void readROIXML(const char *file_name);
	void readROIOffsetXML(const char *file_name);
	void readLightXML(char data[], size_t len);
	void readWeightXML(char data[], size_t len);
	void readSeriesXML(char data[], size_t len);
	void readROIXML(char data[], size_t len);
	void readROIOffsetXML(char data[], size_t len);
	void config(void* ptr);
	bool roi_valid(float rg, float bg, vector<float> &valid_gray, vector<float> &valid_blue, vector<float> &valid_yellow);
    float distance(float rg, float bg, float stdA, float stdB);
	void ROIProcess(vector<float> &valid_color, float color_gain[2]);
    void serial_filter(float final_gain[2]);
    void estimate();
	AWBAlgo(int lscsz_);
	~AWBAlgo();

    float weights[4];
    float whitepoints[8];
    float ROIList[8];
    float ROIGrayList[8];
    float ROIBlueList[8];
    float ROIYellowList[8];
    float ROIGrayOffset[3]; // rg/bg/weight
    float ROIBlueOffset[3];
    float ROIYellowOffset[3];
    float serial_coef[5];
    float final_gain[2];
    vector<float> gain_series;
	char verbose;
	int* stats;
	int blknum;

	float* ccm_list[4] = { high_ccm, D65_ccm, tl84_ccm, A_ccm };
	float final_ccm[9];
	float high_ccm[9] = { 1.354,-0.391,0.048,
	-0.180,1.305,-0.125,
	-0.0169,-0.304,1.321 };
	float D65_ccm[9] = { 1.354,-0.391,0.048,
	-0.180,1.305,-0.125,
	-0.0169,-0.304,1.321 };
	float cwf_ccm[9] = { 1.32, -0.27, -0.05,
	-0.25, 1.32, -0.07,
	0.05, -0.4, 1.35 };
	float tl84_ccm[9] = { 1.32, -0.27, -0.05,
	-0.25, 1.33, -0.08,
	0.05, -0.4, 1.35 };
	float A_ccm[9] = { 1.22, -0.2, -0.02,
	-0.37, 1.37, 0.0,
	0.1, -0.66, 1.56 };

	int lscsz;
	float* table_buffer;
	float* lsctable[4];
	float* final_lsctable;

private:

    typedef pair<int, float> PAIR;

    struct cmpByValue
    {
        bool operator()(const PAIR& v1, const PAIR& v2)
        {
            return v1.second < v2.second;
        }
    };
};









