//#include <iostream>
//#include <fstream>
//#include "nlohmann/json.hpp"
//
//using namespace std;
//using json = nlohmann::json;
//
//static int val;
//static string valstr;
//static int lut[4096], lutsz;
//
//static void parse_array(string& src, vector<int>& res)
//{
//	res.clear();
//	int i = 0, n = src.length();
//	while (i < n && (src[i] < '0' || src[i] > '9'))
//		i++;
//	while (i < n)
//	{
//		int j = i;
//		while (i < n && (src[i] >= '0' && src[i] <= '9'))
//			i++;
//		res.push_back(stoi(src.substr(j, i - j)));
//		while (i < n && (src[i] < '0' || src[i] > '9'))
//			i++;
//	}
//}
//
//int main(int argc, char* argv[])
//{
//	std::ifstream jsonfile("Raw_Bayer_TNR.json");
//	json cfg = json::parse(jsonfile);
//
//	if (cfg.contains("/Parameters/0/MAE/Kernel_Size/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/MAE/Kernel_Size/Value"_json_pointer];
//		cout << "MAE/Kernel_Size: " << val << "\n";
//		//t3DNR_PARA.m_ucYMAEKernelSize = val;
//	}
//	if (cfg.contains("/Parameters/0/Spatial Filter/Blending_Weight/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Spatial Filter/Blending_Weight/Value"_json_pointer];
//		cout << "Spatial Filter/Blending_Weight: " << val << "\n";
//		//t3DNR_PARA.m_ucOriAvgBlendingWeight = val;
//	}
//	if (cfg.contains("/Parameters/0/Spatial Filter/Filter_Th_R/Value/0"_json_pointer))
//	{
//		lutsz = 6;
//		valstr = cfg["/Parameters/0/Spatial Filter/Filter_Th_R/Value/0"_json_pointer];
//		cout << "Spatial Filter/Filter_Th_R: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_udAltekTh_L_R[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Spatial Filter/Filter_Th_G/Value/0"_json_pointer))
//	{
//		lutsz = 6;
//		valstr = cfg["/Parameters/0/Spatial Filter/Filter_Th_G/Value/0"_json_pointer];
//		cout << "Spatial Filter/Filter_Th_G: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_udAltekTh_L_G[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Spatial Filter/Filter_Th_B/Value/0"_json_pointer))
//	{
//		lutsz = 6;
//		valstr = cfg["/Parameters/0/Spatial Filter/Filter_Th_B/Value/0"_json_pointer];
//		cout << "Spatial Filter/Filter_Th_B: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_udAltekTh_L_B[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Spatial Filter/Filter_Th_Power/Value/0"_json_pointer))
//	{
//		lutsz = 9;
//		valstr = cfg["/Parameters/0/Spatial Filter/MAE_Preserve/Value/0"_json_pointer];
//		cout << "Spatial Filter/MAE_Preserve: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_pudAltekTh_Power[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Mask/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Mask/Value"_json_pointer];
//		cout << "Confidence/Mask: " << val << "\n";
//		//t3DNR_PARA.m_ucConfidenceMask = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/STD Lookup Mode/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/STD Lookup Mode/Value"_json_pointer];
//		cout << "Confidence/STD Lookup Mode: " << val << "\n";
//		//t3DNR_PARA.m_ucSelStdLookupIntensity = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/STD R/Value/0"_json_pointer))
//	{
//		lutsz = 41;
//		valstr = cfg["/Parameters/0/Confidence/STD R/Value/0"_json_pointer];
//		cout << "Confidence/STD R: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_uwConfidenceStd_R[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/STD G/Value/0"_json_pointer))
//	{
//		lutsz = 41;
//		valstr = cfg["/Parameters/0/Confidence/STD G/Value/0"_json_pointer];
//		cout << "Confidence/STD G: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_uwConfidenceStd_G[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/STD B/Value/0"_json_pointer))
//	{
//		lutsz = 41;
//		valstr = cfg["/Parameters/0/Confidence/STD B/Value/0"_json_pointer];
//		cout << "Confidence/STD B: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_uwConfidenceStd_B[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Color Difference Mode/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Color Difference Mode/Value"_json_pointer];
//		cout << "Confidence/Color Difference Mode: " << val << "\n";
//		//t3DNR_PARA.m_ucColorDiff_Mode = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Discount LUT/Value/0"_json_pointer))
//	{
//		lutsz = 16;
//		valstr = cfg["/Parameters/0/Confidence/Discount LUT/Value/0"_json_pointer];
//		cout << "Confidence/Discount LUT: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucDiscount_LUT[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Discount Gap/Value/0"_json_pointer))
//	{
//		lutsz = 15;
//		valstr = cfg["/Parameters/0/Confidence/Discount Gap/Value/0"_json_pointer];
//		cout << "Confidence/Discount Gap: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_Discount_Gap[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Color Difference Gap/Value/0"_json_pointer))
//	{
//		lutsz = 26;
//		valstr = cfg["/Parameters/0/Confidence/Color Difference Gap/Value/0"_json_pointer];
//		cout << "Confidence/Color Difference Gap: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_Colordiff_Var_Gap[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Anti Ghost/Value/0"_json_pointer))
//	{
//		lutsz = 27;
//		valstr = cfg["/Parameters/0/Confidence/Anti Ghost/Value/0"_json_pointer];
//		cout << "Confidence/Anti Ghost: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucAntiGhost_Confid[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Remapping Weight/Value/0"_json_pointer))
//	{
//		lutsz = 2;
//		valstr = cfg["/Parameters/0/Confidence/Remapping Weight/Value/0"_json_pointer];
//		cout << "Confidence/Remapping Weight: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucTFRatioWeight[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Scale/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Scale/Value"_json_pointer];
//		cout << "Confidence/Scale: " << val << "\n";
//		//t3DNR_PARA.m_ucConfidence_Scale = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Threshold/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Threshold/Value"_json_pointer];
//		cout << "Confidence/Threshold: " << val << "\n";
//		//t3DNR_PARA.m_ucConfidence_Th = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Variance_Mode/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Variance_Mode/Value"_json_pointer];
//		cout << "Confidence/Variance_Mode: " << val << "\n";
//		//t3DNR_PARA.m_ucUseCalibrateSigmaOnly = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Blending_Mode/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Blending_Mode/Value"_json_pointer];
//		cout << "Confidence/Blending_Mode: " << val << "\n";
//		//t3DNR_PARA.m_ucUseCalibrateSigmaOnly = val;
//	}
//	if (cfg.contains("/Parameters/0/Confidence/Blending_Mask/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Confidence/Blending_Mask/Value"_json_pointer];
//		cout << "Confidence/Blending_Mask: " << val << "\n";
//		//t3DNR_PARA.m_ucMinMaxAvgMask = val;
//	}
//	if (cfg.contains("/Parameters/0/Temporal Filter/Frame Weight Lut/Value/0"_json_pointer))
//	{
//		lutsz = 16;
//		valstr = cfg["/Parameters/0/Temporal Filter/Frame Weight Lut/Value/0"_json_pointer];
//		cout << "Temporal Filter/Frame Weight Lut: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//t3DNR_PARA.m_ucTemporalFrameWeight = lut[0];
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/Frame Weight Lut/Value/0"_json_pointer))
//	{
//		lutsz = 16;
//		valstr = cfg["/Parameters/0/Local Refine/Frame Weight Lut/Value/0"_json_pointer];
//		cout << "Local Refine/Frame Weight Lut: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//t3DNR_PARA.m_ucResidueFrameWeight = lut[0];
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/Patch Diff Gap/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Local Refine/Patch Diff Gap/Value"_json_pointer];
//		cout << "Local Refine/Patch Diff Gap: " << val << "\n";
//		//t3DNR_PARA.m_ucPatchDiffGap = val;
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/KwSMulDiv/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Local Refine/KwSMulDiv/Value"_json_pointer];
//		cout << "Local Refine/KwSMulDiv: " << val << "\n";
//		//t3DNR_PARA.m_uwKwMulDiv = val;
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/KwShDiv/Value"_json_pointer))
//	{
//		val = cfg["/Parameters/0/Local Refine/KwShDiv/Value"_json_pointer];
//		cout << "Local Refine/KwShDiv: " << val << "\n";
//		//t3DNR_PARA.m_ucKwShDiv = val;
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/KwpLUT/Value/0"_json_pointer))
//	{
//		lutsz = 3;
//		valstr = cfg["/Parameters/0/Local Refine/KwpLUT/Value/0"_json_pointer];
//		cout << "Local Refine/KwpLUT: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucKWPLUT[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/Gwp_Lut/Value/0"_json_pointer))
//	{
//		lutsz = 32;
//		valstr = cfg["/Parameters/0/Local Refine/Gwp_Lut/Value/0"_json_pointer];
//		cout << "Local Refine/Gwp_Lut: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucGWP_LUT[lutindex] = lut[lutindex];
//	}
//	if (cfg.contains("/Parameters/0/Local Refine/Ratio Weight/Value/0"_json_pointer))
//	{
//		lutsz = 2;
//		valstr = cfg["/Parameters/0/Local Refine/Ratio Weight/Value/0"_json_pointer];
//		cout << "Local Refine/Ratio Weight: " << valstr << "\n";
//		vector<int> lut;
//		parse_array(valstr, lut);
//		assert(lutsz == lut.size());
//		//for (int lutindex = 0; lutindex < lutsz; lutindex++)
//		//	t3DNR_PARA.m_ucLRTNRRatioWeight[lutindex] = lut[lutindex];
//	}
//	return 0;
//}
//
//
//
//
