//#include <opencv2/highgui.hpp>
//#include <string>
//#include <vector>
//#include <ctime>
//#include "../RawToBGR/gxr_softisp.hpp"
//
//using std::vector;
//using std::string;
//
//static char const* RawPattName[4] =
//{
//	"RG/GB",
//	"BG/GR",
//	"GR/BG",
//	"GB/RG",
//};
//
//static int number[2] = { 0, 0 };
//static string outpre;
//static cv::Size size, roisz, lscsz, blksz;
//static int rpat = GXR_Raw_Patt_Impl;
//static int lsc = 1;
//static int group = 1;
//static cv::Vec4f wb(1.f, 1.f, 1.f, 1.f);
//static cv::Mat lsctable, src, dst, awb_blockstats, ae_blockstats, histgram;
//static char key[512], valstr[512]; int ninput;
//static float ccm[9];
//static ushort gmtablex[257] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,104,108,112,116,120,124,128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,256,260,264,268,272,276,280,284,288,292,296,300,304,308,312,316,320,324,328,332,336,340,344,348,352,356,360,364,368,372,376,380,384,388,392,396,400,404,408,412,416,420,424,428,432,436,440,444,448,452,456,460,464,468,472,476,480,484,488,492,496,500,504,508,512,516,520,524,528,532,536,540,544,548,552,556,560,564,568,572,576,580,584,588,592,596,600,604,608,612,616,620,624,628,632,636,640,644,648,652,656,660,664,668,672,676,680,684,688,692,696,700,704,708,712,716,720,724,728,732,736,740,744,748,752,756,760,764,768,772,776,780,784,788,792,796,800,804,808,812,816,820,824,828,832,836,840,844,848,852,856,860,864,868,872,876,880,884,888,892,896,900,904,908,912,916,920,924,928,932,936,940,944,948,952,956,960,964,968,972,976,980,984,988,992,996,1000,1004,1008,1012,1016,1020,1023};
//static ushort gmtabley[257] = {0,51,87,113,135,153,170,185,199,211,223,235,245,256,265,275,284,293,301,309,317,325,332,339,346,353,360,367,373,379,386,392,398,404,409,415,421,426,431,437,442,447,452,457,462,467,472,477,481,486,490,495,499,504,508,513,517,521,525,530,534,538,542,546,550,554,558,561,565,569,573,576,580,584,587,591,595,598,602,605,609,612,616,619,622,626,629,632,636,639,642,645,649,652,655,658,661,664,667,670,674,677,680,683,686,689,691,694,697,700,703,706,709,712,714,717,720,723,726,728,731,734,737,739,742,745,747,750,753,755,758,760,763,766,768,771,773,776,778,781,783,786,788,791,793,796,798,801,803,805,808,810,813,815,817,820,822,824,827,829,831,834,836,838,841,843,845,847,850,852,854,856,859,861,863,865,867,870,872,874,876,878,880,883,885,887,889,891,893,895,897,899,901,904,906,908,910,912,914,916,918,920,922,924,926,928,930,932,934,936,938,940,942,944,946,948,950,952,954,955,957,959,961,963,965,967,969,971,973,974,976,978,980,982,984,986,987,989,991,993,995,997,998,1000,1002,1004,1006,1007,1009,1011,1013,1015,1016,1018,1020,1022,1023};
//static ushort gmtalbex_pre[257] = {0};
//static ushort gmtalbey_pre[257] = {0};
//static int first = 1;
//static GXR_Softisp::SizeParam SZPARAM, SZPARAM_pre;
//static GXR_Softisp::Config CFG, CFG_pre;
//static GXR_Softisp::Input SRC;
//static GXR_Softisp::Output DST;
//static char outext[32] = ".bmp";
//
//static void load_lsc(FILE* fid)
//{
//	_strlwr(key);
//	lsctable.create(4, lscsz.height * lscsz.width, CV_32FC1);
//	int row = -1;
//	if (strstr(key, "lsc table r"))
//		row = 0;
//	else if (strstr(key, "lsc table gr"))
//		row = 1;
//	else if (strstr(key, "lsc table gb"))
//		row = 2;
//	else if (strstr(key, "lsc table b"))
//		row = 3;
//	CV_Assert(row != -1);
//	float* L = lsctable.ptr<float>(row);
//	for (int w = 0; w < lsctable.cols; ++w)
//		fscanf(fid, " %f,", L + w);
//	fgetc(fid);
//}
//
//static void load_gamma(FILE * fid)
//{
//	ushort* G = NULL;
//	if (strstr(key, "gamma table x"))
//		G = gmtablex;
//	else if (strstr(key, "gamma table y"))
//		G = gmtabley;
//	assert(G);
//	for (int w = 0; w < 257; ++w)
//		fscanf(fid, " %hu,", G + w);
//	fgetc(fid);
//}
//
//static void load_rawtxt()
//{
//	FILE* fid = fopen(valstr, "r");
//	if (fid == NULL)
//	{
//		fprintf(stdout, "no file named %s\n", valstr);
//		return;
//	}
//	while (!feof(fid))
//	{
//		ninput = fscanf(fid, " %511[^:\n]%*[:]%511[^;\n]%*[^\n]", key, valstr);
//		_strlwr(key);
//		if (key[0] == ';') continue;
//		if (key[0] == '[') continue;
//		if (strstr(key, "lsc table"))
//			load_lsc(fid);
//		if (strstr(key, "gamma table"))
//			load_gamma(fid);
//		if (ninput != 2) continue;
//		float val = static_cast<float>(atof(valstr));
//		int ival = static_cast<int>(val);
//		if (!strcmp(key, "lsc_width"))
//		{
//			lscsz.width = ival;
//			SZPARAM.lsc_width = lscsz.width;
//		}
//		else if (!strcmp(key, "lsc_height"))
//		{
//			lscsz.height = ival;
//			SZPARAM.lsc_height = lscsz.height;
//		}
//	}
//	fclose(fid);
//}
//
//void convert_pattern()
//{
//	rpat = GXR_Raw_Patt_Impl;
//	for (int i = 0; i < 4; ++i)
//		valstr[i] = static_cast<char>(tolower(valstr[i]));
//	if (!strncmp(valstr, "rggb", 4))
//		rpat = 0;
//	else if (!strncmp(valstr, "bggr", 4))
//		rpat = 1;
//	else if (!strncmp(valstr, "grbg", 4))
//		rpat = 2;
//	else if (!strncmp(valstr, "gbrg", 4))
//		rpat = 3;
//	else
//		rpat = atoi(valstr);
//}
//
//int main(int argc, char** argv)
//{
//	char const* inidefault = "this.ini";
//	char const* ini = (argc > 1) ? argv[1] : inidefault;
//	if (argc > 1 && !strcmp(argv[1], "/?"))
//	{
//		fprintf(stdout, "%s inifile\n", argv[0]);
//		return 1;
//	}
//	outext[sizeof(outext) - 1] = '\0';
//	GXR_Softisp isp;
//	string version = isp.get_version().Version;
//	histgram.create(1, 1 << 10, CV_32SC1);
//	FILE* fid = fopen(ini, "r");
//	double invfreq = 1e3 / cv::getTickFrequency();
//	int64_t sumtest = cv::getTickCount();
//	int64_t sumproc = 0;
//	if (fid == NULL)
//	{
//		fprintf(stdout, "no file named %s\n", ini);
//		return 2;
//	}
//	while (!feof(fid))
//	{
//		fscanf(fid, " %511[^\n]%*[^\n]", key);
//		if (key[0] != '[') continue;
//		if (!strcmp(key, "[RawFiles]"))
//			break;
//	}
//
//	memset(&SZPARAM, 0, sizeof(SZPARAM));
//	memset(&CFG, 0, sizeof(CFG));
//	memset(&SRC, 0, sizeof(SRC));
//	SRC.top = -1; SRC.left = -1;
//	while (!feof(fid))
//	{
//		ninput = fscanf(fid, " %511[^=\n]=%511[^;\n]%*[^\n]", key, valstr);
//		if (key[0] == ';') continue;
//		if (key[0] == '[') break;
//		double val = atof(valstr);
//		if (ninput != 2)
//			continue;
//		else if (!strcmp(key, "size"))
//		{
//			sscanf(valstr, "%dx%d", &(size.width), &(size.height));
//			SZPARAM.org_width = size.width;
//			SZPARAM.org_height = size.height;
//		}
//		else if (!strcmp(key, "roisz"))
//		{
//			sscanf(valstr, "%dx%d", &(roisz.width), &(roisz.height));
//			SZPARAM.roi_width = roisz.width;
//			SZPARAM.roi_height = roisz.height;
//		}
//		else if (!strcmp(key, "lscsz"))
//		{
//			sscanf(valstr, "%dx%d", &(lscsz.width), &(lscsz.height));
//			SZPARAM.lsc_width = lscsz.width;
//			SZPARAM.lsc_height = lscsz.height;
//		}
//		else if (!strcmp(key, "blksz"))
//		{
//			sscanf(valstr, "%dx%d", &(blksz.width), &(blksz.height));
//			SZPARAM.block_width = blksz.width;
//			SZPARAM.block_height = blksz.height;
//		}
//		else if (!strcmp(key, "lsc"))
//			lsc = static_cast<int>(val);
//		else if (!strcmp(key, "group"))
//			group = static_cast<int>(val);
//		else if (!strcmp(key, "verbose"))
//			CFG.verbose = static_cast<int>(val);
//		else if (!strcmp(key, "nthread"))
//			CFG.nthread = static_cast<int>(val);
//		else if (!strcmp(key, "wben"))
//			CFG.wben = static_cast<int>(val);
//		else if (!strcmp(key, "expen"))
//			CFG.expen = static_cast<int>(val);
//		else if (!strcmp(key, "aesat"))
//			CFG.aesat = static_cast<int>(val);
//		else if (!strcmp(key, "ynoise1"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise, CFG.ynoise + 1, CFG.ynoise + 2, CFG.ynoise + 3, CFG.ynoise + 4);
//		else if (!strcmp(key, "ynoise2"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 5, CFG.ynoise + 6, CFG.ynoise + 7, CFG.ynoise + 8, CFG.ynoise + 9);
//		else if (!strcmp(key, "ynoise3"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 10, CFG.ynoise + 11, CFG.ynoise + 12, CFG.ynoise + 13, CFG.ynoise + 14);
//		else if (!strcmp(key, "ynoise4"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 15, CFG.ynoise + 16, CFG.ynoise + 17, CFG.ynoise + 18, CFG.ynoise + 19);
//		else if (!strcmp(key, "ynoise5"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 20, CFG.ynoise + 21, CFG.ynoise + 22, CFG.ynoise + 23, CFG.ynoise + 24);
//		else if (!strcmp(key, "ynoise6"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 25, CFG.ynoise + 26, CFG.ynoise + 27, CFG.ynoise + 28, CFG.ynoise + 29);
//		else if (!strcmp(key, "ynoise7"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 30, CFG.ynoise + 31, CFG.ynoise + 32, CFG.ynoise + 33, CFG.ynoise + 34);
//		else if (!strcmp(key, "ynoise8"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.ynoise + 35, CFG.ynoise + 36, CFG.ynoise + 37, CFG.ynoise + 38, CFG.ynoise + 39);
//		else if (!strcmp(key, "cnoise1"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise, CFG.cnoise + 1, CFG.cnoise + 2, CFG.cnoise + 3, CFG.cnoise + 4);
//		else if (!strcmp(key, "cnoise2"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 5, CFG.cnoise + 6, CFG.cnoise + 7, CFG.cnoise + 8, CFG.cnoise + 9);
//		else if (!strcmp(key, "cnoise3"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 10, CFG.cnoise + 11, CFG.cnoise + 12, CFG.cnoise + 13, CFG.cnoise + 14);
//		else if (!strcmp(key, "cnoise4"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 15, CFG.cnoise + 16, CFG.cnoise + 17, CFG.cnoise + 18, CFG.cnoise + 19);
//		else if (!strcmp(key, "cnoise5"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 20, CFG.cnoise + 21, CFG.cnoise + 22, CFG.cnoise + 23, CFG.cnoise + 24);
//		else if (!strcmp(key, "cnoise6"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 25, CFG.cnoise + 26, CFG.cnoise + 27, CFG.cnoise + 28, CFG.cnoise + 29);
//		else if (!strcmp(key, "cnoise7"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 30, CFG.cnoise + 31, CFG.cnoise + 32, CFG.cnoise + 33, CFG.cnoise + 34);
//		else if (!strcmp(key, "cnoise8"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.cnoise + 35, CFG.cnoise + 36, CFG.cnoise + 37, CFG.cnoise + 38, CFG.cnoise + 39);
//		else if (!strcmp(key, "sharp1"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp, CFG.sharp + 1, CFG.sharp + 2, CFG.sharp + 3, CFG.sharp + 4);
//		else if (!strcmp(key, "sharp2"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 5, CFG.sharp + 6, CFG.sharp + 7, CFG.sharp + 8, CFG.sharp + 9);
//		else if (!strcmp(key, "sharp3"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 10, CFG.sharp + 11, CFG.sharp + 12, CFG.sharp + 13, CFG.sharp + 14);
//		else if (!strcmp(key, "sharp4"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 15, CFG.sharp + 16, CFG.sharp + 17, CFG.sharp + 18, CFG.sharp + 19);
//		else if (!strcmp(key, "sharp5"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 20, CFG.sharp + 21, CFG.sharp + 22, CFG.sharp + 23, CFG.sharp + 24);
//		else if (!strcmp(key, "sharp6"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 25, CFG.sharp + 26, CFG.sharp + 27, CFG.sharp + 28, CFG.sharp + 29);
//		else if (!strcmp(key, "sharp7"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 30, CFG.sharp + 31, CFG.sharp + 32, CFG.sharp + 33, CFG.sharp + 34);
//		else if (!strcmp(key, "sharp8"))
//			sscanf(valstr, "%f,%f,%f,%f,%f", CFG.sharp + 35, CFG.sharp + 36, CFG.sharp + 37, CFG.sharp + 38, CFG.sharp + 39);
//		else if (!strcmp(key, "bdlarge"))
//			sscanf(valstr, "%f,%f,%f,%f,%f,%f,%f,%f", CFG.bdlarge, CFG.bdlarge + 1, CFG.bdlarge + 2, CFG.bdlarge + 3,
//				CFG.bdlarge + 4, CFG.bdlarge + 5, CFG.bdlarge + 6, CFG.bdlarge + 7);
//		else if (!strcmp(key, "rpat"))
//		{
//			convert_pattern();
//			SRC.rpat = rpat;
//		}
//		else if (!strcmp(key, "depth"))
//			SRC.depth = static_cast<int>(val);
//		else if (!strcmp(key, "blacklevel"))
//			sscanf(valstr, "%d,%d,%d,%d", SRC.blacklevel, SRC.blacklevel + 1, SRC.blacklevel + 2, SRC.blacklevel + 3);
//		else if (!strcmp(key, "wbgain"))
//		{
//			ninput = sscanf(valstr, "%f,%f,%f,%f", wb.val, wb.val + 1, wb.val + 2, wb.val + 3);
//			CV_Assert(ninput == 4);
//			memcpy(SRC.wbgain, wb.val, sizeof(wb));
//		}
//		else if (!strcmp(key, "ccm"))
//		{
//			ninput = sscanf(valstr,
//				"%f,%f,%f,%f,%f,%f,%f,%f,%f",
//				ccm + 0, ccm + 1, ccm + 2,
//				ccm + 3, ccm + 4, ccm + 5,
//				ccm + 6, ccm + 7, ccm + 8);
//			CV_Assert(ninput == 9);
//			memcpy(SRC.ccm, ccm, sizeof(ccm));
//		}
//		else if (!strcmp(key, "sensorgain"))
//			SRC.sensorgain = static_cast<float>(val);
//		else if (!strcmp(key, "ispgain"))
//			SRC.ispgain = static_cast<float>(val);
//		else if (!strcmp(key, "adrcgain"))
//			SRC.adrcgain = static_cast<float>(val);
//		else if (!strcmp(key, "shutter"))
//			SRC.shutter = static_cast<float>(val);
//		else if (!strcmp(key, "luxindex"))
//			SRC.luxindex = static_cast<float>(val);
//		else if (!strcmp(key, "top"))
//			SRC.top = static_cast<int>(val);
//		else if (!strcmp(key, "left"))
//			SRC.left = static_cast<int>(val);
//		else if (!strcmp(key, "text"))
//			load_rawtxt();
//		else if (!strcmp(key, "file"))
//		{
//			CV_Assert(static_cast<unsigned>(rpat) < GXR_Raw_Patt_Impl);
//			//CV_Assert((10 <= depth) && (depth <= 16));
//
//			CFG.gmtablex = gmtablex;
//			CFG.gmtabley = gmtabley;
//			for (int cn = 0; cn < 4; ++cn)
//				SRC.lsctable[cn] = lsctable.ptr<float>(cn);
//			if (!lsc)
//			{
//				for (int cn = 0; cn < 4; ++cn)
//					for (int w = 0; w < lscsz.height * lscsz.width; w++)
//						SRC.lsctable[cn][w] = 1;
//			}
//
//			if (!group)
//			{
//				SZPARAM.roi_height = SZPARAM.org_height;
//				SZPARAM.roi_width = SZPARAM.org_width;
//				SRC.top = 0;
//				SRC.left = 0;
//			}
//			else
//			{
//				if (SZPARAM.roi_height == 0 || SZPARAM.roi_width == 0)
//				{
//					SZPARAM.roi_height = ((SZPARAM.org_height >> 1) >> 5) << 5;
//					SZPARAM.roi_width = ((SZPARAM.org_width >> 1) >> 5) << 5;
//				}
//				if (SRC.top == -1 || SRC.left == -1)
//				{
//					SRC.top = ((SZPARAM.org_height >> 2) >> 6) << 6;
//					SRC.left = ((SZPARAM.org_width >> 2) >> 6) << 6;
//				}
//			}
//
//			if (first || (!memcmp(&SZPARAM, &SZPARAM_pre, sizeof(SZPARAM))))
//			{
//				CV_Assert(!(isp.create(SZPARAM)));
//				memcpy(&SZPARAM_pre, &SZPARAM, sizeof(SZPARAM));
//				int blknum_x, blknum_y;
//				isp.get_blocknum(blknum_x, blknum_y);
//				awb_blockstats.create(blknum_y, blknum_x, CV_32SC3);
//				ae_blockstats.create(blknum_y, blknum_x * 2, CV_32SC4);
//			}
//			if (first || (!memcmp(&CFG, &CFG_pre, sizeof(CFG))) || (!memcmp(gmtablex, gmtalbex_pre, sizeof(gmtablex))) || (!memcmp(gmtabley, gmtalbey_pre, sizeof(gmtabley))))
//			{
//				CV_Assert(!(isp.config(CFG)));
//				memcpy(&CFG_pre, &CFG, sizeof(CFG));
//				memcpy(gmtalbex_pre, gmtablex, sizeof(gmtablex));
//				memcpy(gmtalbey_pre, gmtabley, sizeof(gmtabley));
//			}
//			first = 0;
//
//			FILE* fin = fopen(valstr, "rb");
//			if (!fin)
//			{
//				fprintf(stdout, "%3d: no file named %s\n"
//					"\tand will ignore this file\n", number[0] + number[1], valstr);
//				++(number[1]);
//				ninput = 0;
//				continue;
//			}
//			src.create(size.height, size.width, CV_16UC1);
//			fread(src.data, size.width * sizeof(ushort), size.height, fin);
//			fclose(fin);		
//			SRC.data = (ushort*)src.data;
//			SRC.stats.awb_blockstats = (int*)awb_blockstats.data;
//			SRC.stats.ae_blockstats = (int*)ae_blockstats.data;
//			SRC.stats.histgram = (int*)histgram.data;
//			dst.create(src.rows, src.cols, CV_8UC3);
//			DST.data = dst.data;
//
//			fprintf(stdout, "\n%3d: %s (%d°¡%d) with\n"
//				"\tverbose %d, nthread %d, wben %d, expen %d, aesat %d\n"
//				"\tynoise1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
//				"\tcnoise1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
//				"\tsharp1 %.3f, %.3f, %.3f, %.3f, %.3f\n"
//				"\tbdlarge %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n"
//				"\tRawType %s, depth %d, top %d, left %d\n"
//				"\tblacklevel %d, %d, %d, %d\n"
//				"\twbgain %.3f, %.3f, %.3f, %.3f\n"
//				"\tccm %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n"
//				"\tsensorgain %.3f, ispgain %.3f, adrcgain %.3f, shutter %.3f, luxindex %.3f\n"
//				"\t\tlsc %.3f, %.3f, %.3f, %.3f\n", 
//				number[0] + number[1] + 1, valstr, size.width, size.height,
//				CFG.verbose, CFG.nthread, CFG.wben, CFG.expen, CFG.aesat,
//				CFG.ynoise[0], CFG.ynoise[1], CFG.ynoise[2], CFG.ynoise[3], CFG.ynoise[4],
//				CFG.cnoise[0], CFG.cnoise[1], CFG.cnoise[2], CFG.cnoise[3], CFG.cnoise[4],
//				CFG.sharp[0], CFG.sharp[1], CFG.sharp[2], CFG.sharp[3], CFG.sharp[4],
//				CFG.bdlarge[0], CFG.bdlarge[1], CFG.bdlarge[2], CFG.bdlarge[3], CFG.bdlarge[4], CFG.bdlarge[5], CFG.bdlarge[6], CFG.bdlarge[7],
//				RawPattName[SRC.rpat], SRC.depth, SRC.top, SRC.left, 
//				SRC.blacklevel[0], SRC.blacklevel[1], SRC.blacklevel[2], SRC.blacklevel[3],
//				SRC.wbgain[0], SRC.wbgain[1], SRC.wbgain[2], SRC.wbgain[3],
//				SRC.ccm[0], SRC.ccm[1], SRC.ccm[2], SRC.ccm[3], SRC.ccm[4], SRC.ccm[5], SRC.ccm[6], SRC.ccm[7], SRC.ccm[8],
//				SRC.sensorgain, SRC.ispgain, SRC.adrcgain, SRC.shutter, SRC.luxindex, 
//				SRC.lsctable[0], SRC.lsctable[1], SRC.lsctable[2], SRC.lsctable[3]);
//
//			int64_t curtick = cv::getTickCount();
//			isp.prestats(SRC);
//			isp.poststats(DST);
//			curtick = cv::getTickCount() - curtick;
//
//			string outpre = string(valstr);
//			size_t dot = outpre.find_last_of('.');
//			if (dot > 0 && dot != string::npos)
//				outpre.erase(outpre.begin() + dot, outpre.end());
//			sprintf(key, "_%s", version.c_str());
//			outpre.append(key);
//			cv::imwrite(outpre.append(outext), dst);
//			fprintf(stdout, "write %s\n", outpre.c_str());
//
//			printf("%.3fms, %s\n", curtick* invfreq, valstr);
//			sumproc += curtick;
//			++(number[0]);
//		}
//	}
//	isp.release();
//	sumtest = cv::getTickCount() - sumtest;
//	fclose(fid);
//	printf(
//		"\n========================================\n"
//		"≈‹Õº %.3f ∫¡√Î£¨∆Ωæ˘ %.3f ∫¡√Î£∫≥…π¶ %d  ß∞‹ %d\n",
//		sumtest * invfreq, sumproc* invfreq / number[0],
//		number[0], number[1]);
//}