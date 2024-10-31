#include <opencv2/highgui.hpp>
#include <string>
#include <vector>
#include <ctime>
#include "../RawToBGR/gxr_raw2bgr.h"
#include "../RawToBGR/rawproc.hpp"

using std::vector;
using std::string;

static char const* RawPattName[4] =
{
	"RG/GB",
	"BG/GR",
	"GR/BG",
	"GB/RG",
};

static int number[2] = { 0, 0 };
static vector<string> names;
static string outpre;
static cv::Size size, lscsz;
static int rpat = GXR_Raw_Patt_Impl;
static int depth = 0;
static char lsc = 1;
static char group = 1;
static cv::Vec4f wb(1.f, 1.f, 1.f, 1.f);
static int top = -1, bottom = -1, left = -1, right = -1;
static cv::Mat lsctable, raw, src, dst;
static char key[512], valstr[512]; int ninput;
static float ccm[9]; static float const* rgbcam = nullptr;
static ushort gmtablex[257] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,256,260,264,268,272,276,280,284,288,292,296,300,304,308,312,316,320,324,328,332,336,340,344,348,352,356,360,364,368,372,376,380,384,388,392,396,400,404,408,412,416,420,424,428,432,436,440,444,448,452,456,460,464,468,472,476,480,484,488,492,496,500,504,508,512,516,520,524,528,532,536,540,544,548,552,556,560,564,568,572,576,580,584,588,592,596,600,604,608,612,616,620,624,628,632,636,640,644,648,652,656,660,664,668,672,676,680,684,688,692,696,700,704,708,712,716,720,724,728,732,736,740,744,748,752,756,760,764,768,772,776,780,784,788,792,796,800,804,808,812,816,820,824,828,832,836,840,844,848,852,856,860,864,868,872,876,880,884,888,892,896,900,904,908,912,916,920,924,928,932,936,940,944,948,952,956,960,964,968,972,976,980,984,988,992,996,1000,1004,1008,1012,1016,1020,1023};
static ushort * gmtx = nullptr;
static ushort gmtabley[257] = {0,51,87,113,135,153,170,185,199,211,223,235,245,256,265,275,284,293,301,309,317,325,332,339,346,353,360,367,373,379,386,392,398,404,409,415,421,426,431,437,442,447,452,457,462,467,472,477,481,486,490,495,499,504,508,513,517,521,525,530,534,538,542,546,550,554,558,561,565,569,573,576,580,584,587,591,595,598,602,605,609,612,616,619,622,626,629,632,636,639,642,645,649,652,655,658,661,664,667,670,674,677,680,683,686,689,691,694,697,700,703,706,709,712,714,717,720,723,726,728,731,734,737,739,742,745,747,750,753,755,758,760,763,766,768,771,773,776,778,781,783,786,788,791,793,796,798,801,803,805,808,810,813,815,817,820,822,824,827,829,831,834,836,838,841,843,845,847,850,852,854,856,859,861,863,865,867,870,872,874,876,878,880,883,885,887,889,891,893,895,897,899,901,904,906,908,910,912,914,916,918,920,922,924,926,928,930,932,934,936,938,940,942,944,946,948,950,952,954,955,957,959,961,963,965,967,969,971,973,974,976,978,980,982,984,986,987,989,991,993,995,997,998,1000,1002,1004,1006,1007,1009,1011,1013,1015,1016,1018,1020,1022,1023};
static ushort * gmty = nullptr;
static char outext[32] = ".bmp";

static Vec3f estGain(Mat const& raw, int rpat, int depth, int dark)
{
	//CV_Assert(depth >= 10);
	CV_Assert(((raw.rows | raw.cols) & 1) == 0);
	CV_Assert(raw.type() == CV_16UC1);
	CV_Assert(raw.cols <= 65536);
	CV_Assert(static_cast<unsigned>(rpat) < GXR_Raw_Patt_Impl);
	unsigned pat = gxr::RawFilter[rpat];
	unsigned const lo = 5 << (depth - 10);
	unsigned const hi = std::min(65535, (1 << depth) - 1) - dark - lo;
	Vec3f wb(1.F, 1.F, 1.F);
	double SUM[3] = { 0, 0, 0 };
	int i[4] =
	{
		gxr::fcol(0, 0, pat), gxr::fcol(0, 1, pat),
		gxr::fcol(1, 0, pat), gxr::fcol(1, 1, pat),
	};

	int const center = 512;
	int hstart = ((raw.rows - center) >> 2) << 1;
	int hstop = hstart + center;
	int wstart = ((raw.cols - center) >> 2) << 1;
	int wstop = wstart + center;
	for (int h = hstart; h < hstop; h += 2)
	{
		ushort const* S0 = raw.ptr<ushort>(h);
		ushort const* S1 = raw.ptr<ushort>(h + 1);
		unsigned line[4] = { 0, 0, 0, 0 };
		for (int w = wstart; w < wstop; w += 2)
		{
			int s00 = max(S0[w] - dark, 0);
			int s01 = max(S0[w + 1] - dark, 0);
			int s10 = max(S1[w] - dark, 0);
			int s11 = max(S1[w + 1] - dark, 0);
			if (lo <= s00 && s00 <= hi
				&& lo <= s01 && s01 <= hi
				&& lo <= s10 && s10 <= hi
				&& lo <= s11 && s11 <= hi)
			{
				line[0] += s00; line[1] += s01;
				line[2] += s10; line[3] += s11;
			}
		}
		SUM[i[0]] += line[0]; SUM[i[1]] += line[1];
		SUM[i[2]] += line[2]; SUM[i[3]] += line[3];
	}

	if (SUM[0] > hi && SUM[1] > hi && SUM[2] > hi)
	{
		wb[0] = static_cast<float>(0.5 * SUM[1] / SUM[0]);
		wb[2] = static_cast<float>(0.5 * SUM[1] / SUM[2]);
	}
	return wb;
}

static void load_lsc(FILE* fid)
{
	_strlwr(key);
	lsctable.create(4, lscsz.height * lscsz.width, CV_32FC1);
	int row = -1;
	if (strstr(key, "lsc table r"))
		row = 0;
	else if (strstr(key, "lsc table gr"))
		row = 1;
	else if (strstr(key, "lsc table gb"))
		row = 2;
	else if (strstr(key, "lsc table b"))
		row = 3;
	CV_Assert(row != -1);
	float* L = lsctable.ptr<float>(row);
	cv::Mat lscview = lsctable.row(row).reshape(1, lscsz.height);
	for (int w = 0; w < lsctable.cols; ++w)
		fscanf(fid, " %f,", L + w);
	fgetc(fid);
}

static void load_gamma(FILE * fid)
{
	ushort* G = NULL;
	if (strstr(key, "gamma table x"))
	{
		G = gmtablex;
		gmtx = gmtablex;
	}
	else if (strstr(key, "gamma table y"))
	{
		G = gmtabley;
		gmty = gmtabley;
	}
	assert(G);
	for (int w = 0; w < 257; ++w)
		fscanf(fid, " %hu,", G + w);
	fgetc(fid);
}

static void load_rawtxt(gxr_raw2bgr * raw2 = 0)
{
	FILE* fid = fopen(valstr, "r");
	if (fid == NULL)
	{
		fprintf(stdout, "no file named %s\n", valstr);
		return;
	}
	while (!feof(fid))
	{
		ninput = fscanf(fid, " %511[^:\n]%*[:]%511[^;\n]%*[^\n]", key, valstr);
		_strlwr(key);
		if (key[0] == ';') continue;
		if (key[0] == '[') continue;
		if (strstr(key, "lsc table"))
			load_lsc(fid);
		if (strstr(key, "gamma table"))
			load_gamma(fid);
		if (ninput != 2) continue;
		float val = static_cast<float>(atof(valstr));
		int ival = static_cast<int>(val);
		if (!strcmp(key, "redgain"))
			wb[0] = val;
		else if (!strcmp(key, "bluegain"))
			wb[2] = val;
		else if (!strcmp(key, "sensorgain"))
			raw2->sensorgain = val;
		else if (!strcmp(key, "ispgain"))
			raw2->ispgain = val;
		else if (!strcmp(key, "ccm[0]"))
		{
			ccm[0] = val;
			int num = 8;
			while (num--)
			{
				fscanf(fid, " %511[^:\n]%*[:]%511[^;\n]%*[^\n]", key, valstr);
				val = static_cast<float>(atof(valstr));
				ccm[8 - num] = val;
			}
			rgbcam = ccm;
		}
		else if (!strcmp(key, "shutter"))
			raw2->shutter = val;
		else if (!strcmp(key, "luxindex"))
			raw2->luxindex = val;
		else if (!strcmp(key, "adrc_gain"))
			raw2->adrcgain = val;
		else if (!strcmp(key, "lsc_width"))
			lscsz.width = ival;
		else if (!strcmp(key, "lsc_height"))
			lscsz.height = ival;
	}
	fclose(fid);
}

void convert_pattern()
{
	rpat = GXR_Raw_Patt_Impl;
	for (int i = 0; i < 4; ++i)
		valstr[i] = static_cast<char>(tolower(valstr[i]));
	if (!strncmp(valstr, "rggb", 4))
		rpat = 0;
	else if (!strncmp(valstr, "bggr", 4))
		rpat = 1;
	else if (!strncmp(valstr, "grbg", 4))
		rpat = 2;
	else if (!strncmp(valstr, "gbrg", 4))
		rpat = 3;
	else
		rpat = atoi(valstr);
}

int main(int argc, char** argv)
{
	lscsz.width = 17; lscsz.height = 13;
	lsctable.create(4, lscsz.height * lscsz.width, CV_32FC1);
	int total = lscsz.height * lscsz.width * 4;
	float* T = lsctable.ptr <float>();
	for (int i = 0; i < total; i++)
		T[i] = 1;
	char const* inidefault = "this.ini";
	char const* ini = (argc > 1) ? argv[1] : inidefault;
	if (argc > 1 && !strcmp(argv[1], "/?"))
	{
		fprintf(stdout, "%s inifile\n", argv[0]);
		return 1;
	}
	outext[sizeof(outext) - 1] = '\0';
	gxr_raw2bgr raw2;
	gxr_raw2bgr_initialize(&raw2);
	string version = gxr_raw2bgr_version().Version;
	FILE* fid = fopen(ini, "r");
	if (fid == NULL)
	{
		fprintf(stdout, "no file named %s\n", ini);
		return 2;
	}
	while (!feof(fid))
	{
		fscanf(fid, " %511[^\n]%*[^\n]", key);
		if (key[0] != '[') continue;
		if (!strcmp(key, "[RawFiles]"))
			break;
	}

	int64_t tick = cv::getTickCount();
	while (!feof(fid))
	{
		ninput = fscanf(fid, " %511[^=\n]=%511[^;\n]%*[^\n]", key, valstr);
		if (key[0] == ';') continue;
		if (key[0] == '[') break;
		double val = atof(valstr);
		if (ninput != 2)
			continue;
		else if (!strcmp(key, "verbose"))
			raw2.verbose = static_cast<char>(val);
		else if (!strcmp(key, "outext"))
			strncpy(outext, valstr, sizeof(outext) - 1);
		else if (!strcmp(key, "size"))
			sscanf(valstr, "%dx%d", &(size.width), &(size.height));
		else if (!strcmp(key, "rpat"))
			convert_pattern();
		else if (!strcmp(key, "depth"))
			depth = static_cast<int>(val);
		else if (!strcmp(key, "blacklevel"))
			sscanf(valstr, "%d,%d,%d,%d", raw2.blacklevel, raw2.blacklevel + 1, raw2.blacklevel + 2, raw2.blacklevel + 3);
		else if (!strcmp(key, "sensorgain"))
			raw2.sensorgain = static_cast<float>(val);
		else if (!strcmp(key, "ispgain"))
			raw2.ispgain = static_cast<float>(val);
		else if (!strcmp(key, "adrcgain"))
			raw2.adrcgain = static_cast<float>(val);
		else if (!strcmp(key, "shutter"))
			raw2.shutter = static_cast<float>(val);
		else if (!strcmp(key, "luxindex"))
			raw2.luxindex = static_cast<float>(val);
		else if (!strcmp(key, "ccm"))
		{
			ninput = sscanf(valstr,
				"%f,%f,%f,%f,%f,%f,%f,%f,%f",
				ccm + 0, ccm + 1, ccm + 2,
				ccm + 3, ccm + 4, ccm + 5,
				ccm + 6, ccm + 7, ccm + 8);
			rgbcam = (ninput == 9 ? ccm : nullptr);
		}
		else if (!strcmp(key, "wbgain"))
		{
			ninput = sscanf(valstr, "%f,%f,%f,%f", wb.val, wb.val + 1, wb.val + 2, wb.val + 3);
			if (ninput != 4)
				wb[0] = 0.F;
		}
		else if (!strcmp(key, "wben"))
			raw2.wben = static_cast<char>(val);
		else if (!strcmp(key, "expen"))
			raw2.expen = static_cast<char>(val);
		else if (!strcmp(key, "aesat"))
			raw2.aesat = static_cast<int>(val);
		else if (!strcmp(key, "bpc"))
			raw2.bpc = static_cast<char>(val);
		else if (!strcmp(key, "lsc"))
			lsc = static_cast<char>(val);
		else if (!strcmp(key, "group"))
			group = static_cast<char>(val);
		else if (!strcmp(key, "ynoise1"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise, raw2.ynoise + 1, raw2.ynoise + 2, raw2.ynoise + 3, raw2.ynoise + 4);
		else if (!strcmp(key, "ynoise2"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 5, raw2.ynoise + 6, raw2.ynoise + 7, raw2.ynoise + 8, raw2.ynoise + 9);
		else if (!strcmp(key, "ynoise3"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 10, raw2.ynoise + 11, raw2.ynoise + 12, raw2.ynoise + 13, raw2.ynoise + 14);
		else if (!strcmp(key, "ynoise4"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 15, raw2.ynoise + 16, raw2.ynoise + 17, raw2.ynoise + 18, raw2.ynoise + 19);
		else if (!strcmp(key, "ynoise5"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 20, raw2.ynoise + 21, raw2.ynoise + 22, raw2.ynoise + 23, raw2.ynoise + 24);
		else if (!strcmp(key, "ynoise6"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 25, raw2.ynoise + 26, raw2.ynoise + 27, raw2.ynoise + 28, raw2.ynoise + 29);
		else if (!strcmp(key, "ynoise7"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 30, raw2.ynoise + 31, raw2.ynoise + 32, raw2.ynoise + 33, raw2.ynoise + 34);
		else if (!strcmp(key, "ynoise8"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.ynoise + 35, raw2.ynoise + 36, raw2.ynoise + 37, raw2.ynoise + 38, raw2.ynoise + 39);
		else if (!strcmp(key, "cnoise1"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise, raw2.cnoise + 1, raw2.cnoise + 2, raw2.cnoise + 3, raw2.cnoise + 4);
		else if (!strcmp(key, "cnoise2"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 5, raw2.cnoise + 6, raw2.cnoise + 7, raw2.cnoise + 8, raw2.cnoise + 9);
		else if (!strcmp(key, "cnoise3"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 10, raw2.cnoise + 11, raw2.cnoise + 12, raw2.cnoise + 13, raw2.cnoise + 14);
		else if (!strcmp(key, "cnoise4"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 15, raw2.cnoise + 16, raw2.cnoise + 17, raw2.cnoise + 18, raw2.cnoise + 19);
		else if (!strcmp(key, "cnoise5"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 20, raw2.cnoise + 21, raw2.cnoise + 22, raw2.cnoise + 23, raw2.cnoise + 24);
		else if (!strcmp(key, "cnoise6"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 25, raw2.cnoise + 26, raw2.cnoise + 27, raw2.cnoise + 28, raw2.cnoise + 29);
		else if (!strcmp(key, "cnoise7"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 30, raw2.cnoise + 31, raw2.cnoise + 32, raw2.cnoise + 33, raw2.cnoise + 34);
		else if (!strcmp(key, "cnoise8"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.cnoise + 35, raw2.cnoise + 36, raw2.cnoise + 37, raw2.cnoise + 38, raw2.cnoise + 39);
		else if (!strcmp(key, "sharp1"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp, raw2.sharp + 1, raw2.sharp + 2, raw2.sharp + 3, raw2.sharp + 4);
		else if (!strcmp(key, "sharp2"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 5, raw2.sharp + 6, raw2.sharp + 7, raw2.sharp + 8, raw2.sharp + 9);
		else if (!strcmp(key, "sharp3"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 10, raw2.sharp + 11, raw2.sharp + 12, raw2.sharp + 13, raw2.sharp + 14);
		else if (!strcmp(key, "sharp4"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 15, raw2.sharp + 16, raw2.sharp + 17, raw2.sharp + 18, raw2.sharp + 19);
		else if (!strcmp(key, "sharp5"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 20, raw2.sharp + 21, raw2.sharp + 22, raw2.sharp + 23, raw2.sharp + 24);
		else if (!strcmp(key, "sharp6"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 25, raw2.sharp + 26, raw2.sharp + 27, raw2.sharp + 28, raw2.sharp + 29);
		else if (!strcmp(key, "sharp7"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 30, raw2.sharp + 31, raw2.sharp + 32, raw2.sharp + 33, raw2.sharp + 34);
		else if (!strcmp(key, "sharp8"))
			sscanf(valstr, "%f,%f,%f,%f,%f", raw2.sharp + 35, raw2.sharp + 36, raw2.sharp + 37, raw2.sharp + 38, raw2.sharp + 39);
		else if (!strcmp(key, "bdlarge"))
			sscanf(valstr, "%f,%f,%f,%f,%f,%f,%f,%f", raw2.bdlarge, raw2.bdlarge + 1, raw2.bdlarge + 2, raw2.bdlarge + 3,
				raw2.bdlarge + 4, raw2.bdlarge + 5, raw2.bdlarge + 6, raw2.bdlarge + 7);
		else if (!strcmp(key, "top"))
			top = static_cast<int>(val);
		else if (!strcmp(key, "bottom"))
			bottom = static_cast<int>(val);
		else if (!strcmp(key, "left"))
			left = static_cast<int>(val);
		else if (!strcmp(key, "right"))
			right = static_cast<int>(val);
		else if (!strcmp(key, "text"))
			load_rawtxt(&raw2);
		else if (!strcmp(key, "file"))
			names.push_back(string(valstr));
		else if (!strcmp(key, "crop"))
		{
			CV_Assert(static_cast<unsigned>(rpat) < GXR_Raw_Patt_Impl);
			//CV_Assert((10 <= depth) && (depth <= 16));
			ninput = static_cast<int>(names.size());

			fprintf(stdout, "\n%3d: processing %d files (%d¡Á%d, %s) with\n"
				"\tverbose %hhd, RawType %s, depth %d\n"
				"\tbpc %d, wben %d, expen %d, aesat %d\n"
				"\tblacklevel %d, %d, %d, %d\n"
				"\twbgain %f, %f, %f, %f\n"
				"\tynoise1 %f, %f, %f, %f, %f\n"
				"\tynoise2 %f, %f, %f, %f, %f\n"
				"\tynoise3 %f, %f, %f, %f, %f\n"
				"\tynoise4 %f, %f, %f, %f, %f\n"
				"\tynoise5 %f, %f, %f, %f, %f\n"
				"\tynoise6 %f, %f, %f, %f, %f\n"
				"\tynoise7 %f, %f, %f, %f, %f\n"
				"\tynoise8 %f, %f, %f, %f, %f\n"
				"\tcnoise1 %f, %f, %f, %f, %f\n"
				"\tcnoise2 %f, %f, %f, %f, %f\n"
				"\tcnoise3 %f, %f, %f, %f, %f\n"
				"\tcnoise4 %f, %f, %f, %f, %f\n"
				"\tcnoise5 %f, %f, %f, %f, %f\n"
				"\tcnoise6 %f, %f, %f, %f, %f\n"
				"\tcnoise7 %f, %f, %f, %f, %f\n"
				"\tcnoise8 %f, %f, %f, %f, %f\n"
				"\tsharp1 %f, %f, %f, %f, %f\n"
				"\tsharp2 %f, %f, %f, %f, %f\n"
				"\tsharp3 %f, %f, %f, %f, %f\n"
				"\tsharp4 %f, %f, %f, %f, %f\n"
				"\tsharp5 %f, %f, %f, %f, %f\n"
				"\tsharp6 %f, %f, %f, %f, %f\n"
				"\tsharp7 %f, %f, %f, %f, %f\n"
				"\tsharp8 %f, %f, %f, %f, %f\n"
				"\tbdlarge %f, %f, %f, %f, %f, %f, %f, %f\n"
				"\tsensorgain %f, ispgain %f\n"
				"\tadrcgain %f, shutter %f, luxindex %f\n",
				number[0] + number[1], ninput, size.width, size.height, outext,
				raw2.verbose, RawPattName[rpat], depth,
				raw2.bpc, raw2.wben, raw2.expen, raw2.aesat,
				raw2.blacklevel[0], raw2.blacklevel[1], raw2.blacklevel[2], raw2.blacklevel[3],
				wb[0], wb[1], wb[2], wb[3],
				raw2.ynoise[0], raw2.ynoise[1], raw2.ynoise[2], raw2.ynoise[3], raw2.ynoise[4],
				raw2.ynoise[5 + 0], raw2.ynoise[5 + 1], raw2.ynoise[5 + 2], raw2.ynoise[5 + 3], raw2.ynoise[5 + 4],
				raw2.ynoise[10 + 0], raw2.ynoise[10 + 1], raw2.ynoise[10 + 2], raw2.ynoise[10 + 3], raw2.ynoise[10 + 4],
				raw2.ynoise[15 + 0], raw2.ynoise[15 + 1], raw2.ynoise[15 + 2], raw2.ynoise[15 + 3], raw2.ynoise[15 + 4],
				raw2.ynoise[20 + 0], raw2.ynoise[20 + 1], raw2.ynoise[20 + 2], raw2.ynoise[20 + 3], raw2.ynoise[20 + 4],
				raw2.ynoise[25 + 0], raw2.ynoise[25 + 1], raw2.ynoise[25 + 2], raw2.ynoise[25 + 3], raw2.ynoise[25 + 4],
				raw2.ynoise[30 + 0], raw2.ynoise[30 + 1], raw2.ynoise[30 + 2], raw2.ynoise[30 + 3], raw2.ynoise[30 + 4],
				raw2.ynoise[35 + 0], raw2.ynoise[35 + 1], raw2.ynoise[35 + 2], raw2.ynoise[35 + 3], raw2.ynoise[35 + 4],
				raw2.cnoise[0], raw2.cnoise[1], raw2.cnoise[2], raw2.cnoise[3], raw2.cnoise[4],
				raw2.cnoise[5 + 0], raw2.cnoise[5 + 1], raw2.cnoise[5 + 2], raw2.cnoise[5 + 3], raw2.cnoise[5 + 4],
				raw2.cnoise[10 + 0], raw2.cnoise[10 + 1], raw2.cnoise[10 + 2], raw2.cnoise[10 + 3], raw2.cnoise[10 + 4],
				raw2.cnoise[15 + 0], raw2.cnoise[15 + 1], raw2.cnoise[15 + 2], raw2.cnoise[15 + 3], raw2.cnoise[15 + 4],
				raw2.cnoise[20 + 0], raw2.cnoise[20 + 1], raw2.cnoise[20 + 2], raw2.cnoise[20 + 3], raw2.cnoise[20 + 4],
				raw2.cnoise[25 + 0], raw2.cnoise[25 + 1], raw2.cnoise[25 + 2], raw2.cnoise[25 + 3], raw2.cnoise[25 + 4],
				raw2.cnoise[30 + 0], raw2.cnoise[30 + 1], raw2.cnoise[30 + 2], raw2.cnoise[30 + 3], raw2.cnoise[30 + 4],
				raw2.cnoise[35 + 0], raw2.cnoise[35 + 1], raw2.cnoise[35 + 2], raw2.cnoise[35 + 3], raw2.cnoise[35 + 4],
				raw2.sharp[0], raw2.sharp[1], raw2.sharp[2], raw2.sharp[3], raw2.sharp[4],
				raw2.sharp[5 + 0], raw2.sharp[5 + 1], raw2.sharp[5 + 2], raw2.sharp[5 + 3], raw2.sharp[5 + 4],
				raw2.sharp[10 + 0], raw2.sharp[10 + 1], raw2.sharp[10 + 2], raw2.sharp[10 + 3], raw2.sharp[10 + 4],
				raw2.sharp[15 + 0], raw2.sharp[15 + 1], raw2.sharp[15 + 2], raw2.sharp[15 + 3], raw2.sharp[15 + 4],
				raw2.sharp[20 + 0], raw2.sharp[20 + 1], raw2.sharp[20 + 2], raw2.sharp[20 + 3], raw2.sharp[20 + 4],
				raw2.sharp[25 + 0], raw2.sharp[25 + 1], raw2.sharp[25 + 2], raw2.sharp[25 + 3], raw2.sharp[25 + 4],
				raw2.sharp[30 + 0], raw2.sharp[30 + 1], raw2.sharp[30 + 2], raw2.sharp[30 + 3], raw2.sharp[30 + 4],
				raw2.sharp[35 + 0], raw2.sharp[35 + 1], raw2.sharp[35 + 2], raw2.sharp[35 + 3], raw2.sharp[35 + 4],
				raw2.bdlarge[0], raw2.bdlarge[1], raw2.bdlarge[2], raw2.bdlarge[3],
				raw2.bdlarge[4], raw2.bdlarge[5], raw2.bdlarge[6], raw2.bdlarge[7],
				raw2.sensorgain, raw2.ispgain, raw2.adrcgain, raw2.shutter, raw2.luxindex);

			if (rgbcam)
			{
				fprintf(stdout, "\tColor correction matrix\n"
					"\t\t%+7f, %+7f, %+7f\n" "\t\t%+7f, %+7f, %+7f\n"
					"\t\t%+7f, %+7f, %+7f\n", ccm[0], ccm[1], ccm[2],
					ccm[3], ccm[4], ccm[5], ccm[6], ccm[7], ccm[8]);
				memcpy(raw2.ccm, ccm, sizeof(ccm));
			}

			for (int i = 0; i < ninput; ++i)
			{
				FILE* fin = fopen(names[i].c_str(), "rb");
				if (!fin)
				{
					fprintf(stdout, "no file named %s\n"
						"\tand will ignore this group\n", names[i].c_str());
					++(number[1]);
					names.clear(); ninput = 0;
					continue;
				}
				raw.create(size.height, size.width, CV_16UC1);
				fread(raw.data, size.width * sizeof(ushort), size.height, fin);
				fclose(fin);
				fprintf(stdout, "read (%*d) %s\n", 1 + (ninput > 9), i, names[i].c_str());
				if (i == 0 && wb[0] == 0.f)
				{
					Vec3f wbbgr = estGain(raw, rpat, depth, raw2.blacklevel[1]);
					wb[0] = wbbgr[2];
					wb[1] = wbbgr[1];
					wb[2] = wbbgr[1];
					wb[3] = wbbgr[0];
				}



				//uchar* imageBuf = (uchar*)(raw.data);
				//size_t bufSize = size.height * size.width * 2;
				//CV_Assert(bufSize % 3 == 0);
				//for (size_t i = 0; i < bufSize; i += 3)
				//{
				//	imageBuf[i + 0] = 127;
				//	imageBuf[i + 1] = 127;
				//	imageBuf[i + 2] = 127;
				//	imageBuf[i + 0] = 255;
				//}



				src = raw;
				raw2.input = (ushort*)src.data;
				raw2.rpat = rpat; raw2.depth = depth;
				raw2.height = size.height, raw2.width = size.width;
				memcpy(raw2.wbgain, wb.val, sizeof(wb));
				if (!group)
				{
					raw2.top = 0;
					raw2.bottom = raw2.height;
					raw2.left = 0;
					raw2.right = raw2.width;
				}
				else if (top < 0)
				{
					raw2.top = ((raw2.height >> 2) >> 6) << 6;
					raw2.bottom = raw2.top + (((raw2.height >> 1) >> 5) << 5);
					raw2.left = ((raw2.width >> 2) >> 6) << 6;
					raw2.right = raw2.left + (((raw2.width >> 1) >> 5) << 5);
				}
				else
				{
					raw2.top = top;
					raw2.bottom = bottom;
					raw2.left = left;
					raw2.right = right;
				}
				raw2.lsc_height = lscsz.height; raw2.lsc_width = lscsz.width;
				raw2.gmtablex = gmtx ? gmtx : gmtablex; raw2.gmtabley = gmty ? gmty : gmtabley;
				for (int cn = 0; cn < 4; ++cn)
					raw2.lsctable[cn] = (float*)lsctable.ptr(cn);
				if (!lsc)
				{
					for (int cn = 0; cn < 4; ++cn)
						for (int w = 0; w < lscsz.height * lscsz.width; w++)
							raw2.lsctable[cn][w] = 1;
				}
				dst.create(src.rows, src.cols, CV_8UC3);
				raw2.output = dst.data;
				gxr_raw2bgr_run_all(&raw2);

				outpre.swap(names[i]);
				size_t dot = outpre.find_last_of('.');
				if (dot > 0 && dot != string::npos)
					outpre.erase(outpre.begin() + dot, outpre.end());
				//sprintf(key, "_%dx%d", (raw2.width + raw2.right - raw2.left) / 2, (raw2.height + raw2.bottom - raw2.top) / 2);
				//outpre.append(key);
				sprintf(key, "_%s", version.c_str());
				outpre.append(key);
				cv::imwrite(outpre.append(outext), dst);
				fprintf(stdout, "write %s\n", outpre.c_str());
			}
			memset(&wb[0], 0, sizeof(wb));
			if (ninput == 0) continue;
			names.clear();
			++(number[0]);
		}
	}
	gxr_raw2bgr_release(&raw2);
	fclose(fid);
	tick = cv::getTickCount() - tick;
	fprintf(stdout, "\n========================================\n"
		"RAW2BGR: %d success(es), %d failure(s) in %.3f seconds\n",
		number[0], number[1], tick / cv::getTickFrequency());
}









