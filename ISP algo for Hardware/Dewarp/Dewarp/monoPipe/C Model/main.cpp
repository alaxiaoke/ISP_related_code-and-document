#include"dewarp_mono.h"

int main()
{
	char tableFile[128] = "DewarpTable_mono1280x1024.raw";
	char inputFile[128] = "input_1280x1024_mono.raw";  //  input
	char outputFile[128] = "dewarp_out_hao_1280x1024_mono.raw"; //  output
	CDewarp_mono Filter;
	// 100
	Filter.m_sCtrlParam.RegEnable = 1;
	Filter.m_sCtrlParam.RegBlockWidth = 4;
	Filter.m_sCtrlParam.RegFullHeight = 1024;
	Filter.m_sCtrlParam.RegFullWidth = 1280;
	Filter.m_sCtrlParam.RegMapWidth = 81;
	Filter.m_sCtrlParam.RegMapHeight = 65;
	Filter.m_sCtrlParam.RegStartH = 36;
	Filter.m_sCtrlParam.RegInputHeight = 955;
	Filter.m_sCtrlParam.RegStartW = 122;
	Filter.m_sCtrlParam.RegInputWidth = 1024;
	Filter.m_sCtrlParam.RegInitLinebuffHeight = 127;

	//90
	//Filter.m_sCtrlParam.RegEnable = 1;
	//Filter.m_sCtrlParam.RegBlockWidth = 5;
	//Filter.m_sCtrlParam.RegFullHeight = 1748;
	//Filter.m_sCtrlParam.RegFullWidth = 2328;
	//Filter.m_sCtrlParam.RegMapWidth = 74;
	//Filter.m_sCtrlParam.RegMapHeight = 56;
	//Filter.m_sCtrlParam.RegStartH = 55;
	//Filter.m_sCtrlParam.RegInputHeight = 1641;
	//Filter.m_sCtrlParam.RegStartW = 0;
	//Filter.m_sCtrlParam.RegInputWidth = 2328;
	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 195;

	//80
	//Filter.m_sCtrlParam.RegEnable = 1;
	//Filter.m_sCtrlParam.RegBlockWidth = 5;
	//Filter.m_sCtrlParam.RegFullHeight = 1748;
	//Filter.m_sCtrlParam.RegFullWidth = 2328;
	//Filter.m_sCtrlParam.RegMapWidth = 74;
	//Filter.m_sCtrlParam.RegMapHeight = 56;
	//Filter.m_sCtrlParam.RegStartH = 49;
	//Filter.m_sCtrlParam.RegInputHeight = 1653;
	//Filter.m_sCtrlParam.RegStartW = 0;
	//Filter.m_sCtrlParam.RegInputWidth = 2328;
	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 173;

	uint16_t* Src_data = NULL;
	uint16_t* Dst_data = NULL;
	uint8_t* DewarpTable = NULL;

	FILE* pf;

	Src_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * sizeof(uint16_t));
	DewarpTable = (uint8_t*)malloc(((Filter.m_sCtrlParam.RegMapWidth << 3) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight * sizeof(uint8_t));

	pf = fopen(tableFile, "rb");
	fread(DewarpTable, 1, ((Filter.m_sCtrlParam.RegMapWidth << 2) + (Filter.m_sCtrlParam.RegMapWidth << 1) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight, pf);
	fclose(pf);
	pf = NULL;

	pf = fopen(inputFile, "rb");
	fread(Src_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;

	Dst_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * sizeof(uint16_t));
	Filter.ProcessImage(Dst_data, Src_data, DewarpTable);
	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
	pf = fopen(outputFile, "wb+");
	fwrite(Dst_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;
	free(Src_data);
	Src_data = NULL;
	free(Dst_data);
	Dst_data = NULL;
	free(DewarpTable);
	DewarpTable = NULL;

	return 1;
}