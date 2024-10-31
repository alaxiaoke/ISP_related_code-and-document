#include"Dewarp.h"

//vrf
//int main(int argc, char** argv)
//{
//	char inputFile[128] = "1.raw";  //  input
//	char outputFile[128] = "2.VRF"; //  output
//
//	int32_t i, j, index;
//	int32_t Height = 1748;
//	int32_t Width = 2328;
//	int32_t Offset = Height * Width;
//	uint16_t* Src_data = NULL;
//	uint16_t* Dst_data = NULL;
//
//	FILE* pf;
//
//	Src_data = (uint16_t*)malloc(Width * Height * 2 * sizeof(uint16_t));
//
//	pf = fopen(inputFile, "rb");
//	fread(Src_data, 1, Width * Height * 2 * sizeof(uint16_t), pf);
//	fclose(pf);
//	pf = NULL;
//
//	Dst_data = (uint16_t*)malloc(Width * Height * sizeof(uint16_t) + 128);
//	for (i = 0; i < Height; i++)
//	{
//		index = i * Width;
//		for (j = 0; j < Width; j++)
//		{
//			Dst_data[index + j] = Src_data[index + j];
//		}
//	}
//	Dst_data[Offset] = Width;//0,1
//	Dst_data[Offset + 1] = Height;//2,3
//	Dst_data[Offset + 2] = 0x0100;//4,5
//	Dst_data[Offset + 3] = 0x1000;//6,7
//	Dst_data[Offset + 4] = Width;//8,9
//	Dst_data[Offset + 5] = 0x80;//10,11
//	Dst_data[Offset + 6] = 0x80;//12,13
//	Dst_data[Offset + 7] = 0x80;//14,15
//	Dst_data[Offset + 8] = 0x80;//16,17
//	Dst_data[Offset + 9] = 0x80;//18,19
//	Dst_data[Offset + 10] = 0x80;//20,21
//	Dst_data[Offset + 11] = 0x80;//22,23
//	Dst_data[Offset + 12] = 0x0010;//24,25
//	Dst_data[Offset + 13] = 0x0010;//26,27
//
//	Dst_data[Offset + 53] = 0x0100;//106,107
//	Dst_data[Offset + 54] = 0x0100;//108,109
//	Dst_data[Offset + 55] = 0x0100;//110,111
//	Dst_data[Offset + 56] = 0x0100;//112,113
//	
//	Dst_data[Offset + 57] = 0x1040;//114,115
//	Dst_data[Offset + 58] = 0x000a;//116,117
//	Dst_data[Offset + 59] = 0x0000;//116,117
//	Dst_data[Offset + 60] = 0x0a00;//120,121
//	Dst_data[Offset + 61] = 0x0040;//122,123
//	Dst_data[Offset + 62] = 0x5629;//124,125
//	Dst_data[Offset + 63] = 0x4652;//126,127
//	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
//	pf = fopen(outputFile, "wb+");
//	fwrite(Dst_data, 1, Width * Height * sizeof(uint16_t) + 128, pf);
//	fclose(pf);
//	pf = NULL;
//	free(Src_data);
//	Src_data = NULL;
//	free(Dst_data);
//	Dst_data = NULL;
//
//	return 1;
//}


//test sigle
int main(int argc, char** argv)
//argv[1]: input file (yuv422 10bit) 
//argv[2]: dewarp table (xx.raw) 
//argv[3]: Output file (yuv422 10bit) 
//argv[4]: Settingfile 
{
	//char tableFile[128] = "DewarpTable_100.raw";
	//char inputFile[128] = "1.raw";  //  input
	//char outputFile[128] = "dewarp_out_1.raw"; //  output
	CDewarp Filter;
	//char Filename[128] = "DewarpSetting.txt";
	char *Filename = argv[4];
	//Filter.SaveSettingFile(Filename);
	Filter.LoadSettingFile(Filename);
	// 100
	//Filter.m_sCtrlParam.RegEnable = 1;
	//Filter.m_sCtrlParam.RegBlockWidth = 5;
	//Filter.m_sCtrlParam.RegFullHeight = 1748;
	//Filter.m_sCtrlParam.RegFullWidth = 2328;
	//Filter.m_sCtrlParam.RegMapWidth = 74;
	//Filter.m_sCtrlParam.RegMapHeight = 56;
	//Filter.m_sCtrlParam.RegStartH = 61;
	//Filter.m_sCtrlParam.RegInputHeight = 1630;
	//Filter.m_sCtrlParam.RegStartW = 226;
	//Filter.m_sCtrlParam.RegInputWidth = 1858;
	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 216;

	//90
	//Filter.m_sCtrlParam.RegEnable = 1;
	//Filter.m_sCtrlParam.RegBlockWidth = 5;
	//Filter.m_sCtrlParam.RegFullHeight = 1748;
	//Filter.m_sCtrlParam.RegFullWidth = 2328;
	//Filter.m_sCtrlParam.RegMapWidth = 74;
	//Filter.m_sCtrlParam.RegMapHeight = 56;
	//Filter.m_sCtrlParam.RegStartH = 55;
	//Filter.m_sCtrlParam.RegInputHeight = 1641;
	//Filter.m_sCtrlParam.RegStartW = 202;
	//Filter.m_sCtrlParam.RegInputWidth = 1906;
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
	//Filter.m_sCtrlParam.RegStartW = 180;
	//Filter.m_sCtrlParam.RegInputWidth = 1954;
	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 173;

	uint16_t* Src_data = NULL;
	uint16_t* Dst_data = NULL;
	uint8_t* DewarpTable = NULL;

	FILE* pf;

	Src_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
	DewarpTable = (uint8_t*)malloc(((Filter.m_sCtrlParam.RegMapWidth << 3) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight * sizeof(uint8_t));

	pf = fopen(argv[2], "rb");
	fread(DewarpTable, 1, ((Filter.m_sCtrlParam.RegMapWidth << 2) + (Filter.m_sCtrlParam.RegMapWidth << 1) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight, pf);
	fclose(pf);
	pf = NULL;

	pf = fopen(argv[1], "rb");
	fread(Src_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;

	Dst_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
	Filter.ProcessImage(Dst_data, Src_data, DewarpTable);
	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
	pf = fopen(argv[3], "wb+");
	fwrite(Dst_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;

	int32_t i, j, index;
	pf = fopen("inputData480x360_yuv422.txt", "wb+");
	for (i = 0; i < Filter.m_sCtrlParam.RegFullHeight*2; i++)
	{
		index = i * Filter.m_sCtrlParam.RegFullWidth;
		for (j = 0; j < Filter.m_sCtrlParam.RegFullWidth; j++)
		{
			fprintf(pf, "%x\n", Src_data[index + j]);
		}
	}
	fclose(pf);
	pf = NULL;

	pf = fopen("OutputData480x360_yuv422.txt", "wb+");
	for (i = 0; i < Filter.m_sCtrlParam.RegFullHeight*2; i++)
	{
		index = i * Filter.m_sCtrlParam.RegFullWidth;
		for (j = 0; j < Filter.m_sCtrlParam.RegFullWidth; j++)
		{
			fprintf(pf, "%x\n", Dst_data[index + j]);
		}
	}
	fclose(pf);
	pf = NULL;

	pf = fopen("DewarpTable480x360.txt", "wb+");
	int32_t tableWidth = (Filter.m_sCtrlParam.RegMapWidth << 2) + (Filter.m_sCtrlParam.RegMapWidth << 1) + (1 << Filter.m_sCtrlParam.RegBlockWidth);
	for (i = 0; i < Filter.m_sCtrlParam.RegMapHeight; i++)
	{
		index = i * tableWidth;
		for (j = 0; j < tableWidth; j++)
		{
			fprintf(pf, "%x\n", DewarpTable[index + j]);
		}
	}
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

//test single image
//int main(int argc, char** argv)
//{
//	char tableFile[128] = "DewarpTable_100.raw";
//	char inputFile[128] = "1.raw";  //  input
//	char outputFile[128] = "dewarp_out_1.raw"; //  output
//	CDewarp Filter;
//	// 100
//	Filter.m_sCtrlParam.RegEnable = 1;
//	Filter.m_sCtrlParam.RegBlockWidth = 5;
//	Filter.m_sCtrlParam.RegFullHeight = 1748;
//	Filter.m_sCtrlParam.RegFullWidth = 2328;
//	Filter.m_sCtrlParam.RegMapWidth = 74;
//	Filter.m_sCtrlParam.RegMapHeight = 56;
//	Filter.m_sCtrlParam.RegStartH = 61;
//	Filter.m_sCtrlParam.RegInputHeight = 1630;
//	Filter.m_sCtrlParam.RegStartW = 226;
//	Filter.m_sCtrlParam.RegInputWidth = 1858;
//	Filter.m_sCtrlParam.RegInitLinebuffHeight = 216;
//
//	//90
//	//Filter.m_sCtrlParam.RegEnable = 1;
//	//Filter.m_sCtrlParam.RegBlockWidth = 5;
//	//Filter.m_sCtrlParam.RegFullHeight = 1748;
//	//Filter.m_sCtrlParam.RegFullWidth = 2328;
//	//Filter.m_sCtrlParam.RegMapWidth = 74;
//	//Filter.m_sCtrlParam.RegMapHeight = 56;
//	//Filter.m_sCtrlParam.RegStartH = 55;
//	//Filter.m_sCtrlParam.RegInputHeight = 1641;
//	//Filter.m_sCtrlParam.RegStartW = 202;
//	//Filter.m_sCtrlParam.RegInputWidth = 1906;
//	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 195;
//
//	//80
//	//Filter.m_sCtrlParam.RegEnable = 1;
//	//Filter.m_sCtrlParam.RegBlockWidth = 5;
//	//Filter.m_sCtrlParam.RegFullHeight = 1748;
//	//Filter.m_sCtrlParam.RegFullWidth = 2328;
//	//Filter.m_sCtrlParam.RegMapWidth = 74;
//	//Filter.m_sCtrlParam.RegMapHeight = 56;
//	//Filter.m_sCtrlParam.RegStartH = 49;
//	//Filter.m_sCtrlParam.RegInputHeight = 1653;
//	//Filter.m_sCtrlParam.RegStartW = 180;
//	//Filter.m_sCtrlParam.RegInputWidth = 1954;
//	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 173;
//
//	uint16_t* Src_data = NULL;
//	uint16_t* Dst_data = NULL;
//	uint8_t* DewarpTable = NULL;
//
//	FILE* pf;
//
//	Src_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
//	DewarpTable = (uint8_t*)malloc(((Filter.m_sCtrlParam.RegMapWidth << 3) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight * sizeof(uint8_t));
//
//	pf = fopen(tableFile, "rb");
//	fread(DewarpTable, 1, ((Filter.m_sCtrlParam.RegMapWidth << 2) + (Filter.m_sCtrlParam.RegMapWidth << 1) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight, pf);
//	fclose(pf);
//	pf = NULL;
//
//	pf = fopen(inputFile, "rb");
//	fread(Src_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
//	fclose(pf);
//	pf = NULL;
//
//	Dst_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
//	Filter.ProcessImage(Dst_data, Src_data, DewarpTable);
//	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
//	pf = fopen(outputFile, "wb+");
//	fwrite(Dst_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
//	fclose(pf);
//	pf = NULL;
//	free(Src_data);
//	Src_data = NULL;
//	free(Dst_data);
//	Dst_data = NULL;
//	free(DewarpTable);
//	DewarpTable = NULL;
//
//	return 1;
//}

//// test video
//int main()
//{
//	char foldIn[128] = "D:/GravityXr/changqing/1109/rawlocalfovpre_ts256nl_mcmap0group0rnd2(2)o_raw/";
//	char foldOut[128] = "D:/GravityXr/changqing/1109/rawlocalfovpre_ts256nl_mcmap0group0rnd2(2)o_raw_out/";
//	char tableFile[128] = "DewarpTable_100.raw";
//	char inputFile[128];// = "1.raw";  //  input
//	char outputFile[128];// = "dewarp_out_1.raw"; //  output
//	char InNum[16];
//	
//
//	CDewarp Filter;
//	// 100
//	Filter.m_sCtrlParam.RegEnable = 1;
//	Filter.m_sCtrlParam.RegBlockWidth = 5;
//	Filter.m_sCtrlParam.RegFullHeight = 1748;
//	Filter.m_sCtrlParam.RegFullWidth = 2328;
//	Filter.m_sCtrlParam.RegMapWidth = 74;
//	Filter.m_sCtrlParam.RegMapHeight = 56;
//	Filter.m_sCtrlParam.RegStartH = 61;
//	Filter.m_sCtrlParam.RegInputHeight = 1630;
//	Filter.m_sCtrlParam.RegStartW = 226;
//	Filter.m_sCtrlParam.RegInputWidth = 1858;
//	Filter.m_sCtrlParam.RegInitLinebuffHeight = 216;
//
//	//90
//	//Filter.m_sCtrlParam.RegEnable = 1;
//	//Filter.m_sCtrlParam.RegBlockWidth = 5;
//	//Filter.m_sCtrlParam.RegFullHeight = 1748;
//	//Filter.m_sCtrlParam.RegFullWidth = 2328;
//	//Filter.m_sCtrlParam.RegMapWidth = 74;
//	//Filter.m_sCtrlParam.RegMapHeight = 56;
//	//Filter.m_sCtrlParam.RegStartH = 55;
//	//Filter.m_sCtrlParam.RegInputHeight = 1641;
//	//Filter.m_sCtrlParam.RegStartW = 202;
//	//Filter.m_sCtrlParam.RegInputWidth = 1906;
//	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 195;
//
//	//80
//	//Filter.m_sCtrlParam.RegEnable = 1;
//	//Filter.m_sCtrlParam.RegBlockWidth = 5;
//	//Filter.m_sCtrlParam.RegFullHeight = 1748;
//	//Filter.m_sCtrlParam.RegFullWidth = 2328;
//	//Filter.m_sCtrlParam.RegMapWidth = 74;
//	//Filter.m_sCtrlParam.RegMapHeight = 56;
//	//Filter.m_sCtrlParam.RegStartH = 49;
//	//Filter.m_sCtrlParam.RegInputHeight = 1653;
//	//Filter.m_sCtrlParam.RegStartW = 180;
//	//Filter.m_sCtrlParam.RegInputWidth = 1954;
//	//Filter.m_sCtrlParam.RegInitLinebuffHeight = 173;
//
//	uint16_t* Src_data = NULL;
//	uint16_t* Dst_data = NULL;
//	uint8_t* DewarpTable = NULL;
//
//	FILE* pf;
//
//	Src_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
//	DewarpTable = (uint8_t*)malloc(((Filter.m_sCtrlParam.RegMapWidth << 3) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight * sizeof(uint8_t));
//
//	pf = fopen(tableFile, "rb");
//	fread(DewarpTable, 1, ((Filter.m_sCtrlParam.RegMapWidth << 2) + (Filter.m_sCtrlParam.RegMapWidth << 1) + (1 << Filter.m_sCtrlParam.RegBlockWidth)) * Filter.m_sCtrlParam.RegMapHeight, pf);
//	fclose(pf);
//	pf = NULL;
//
//	Dst_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t));
//	for (int32_t i = 1; i < 131; i++)
//	{
//		strcpy(inputFile, foldIn);
//		itoa(i, InNum, 10);
//		strcat(inputFile, InNum);
//		strcat(inputFile, ".raw");
//
//		strcpy(outputFile, foldOut);
//		itoa(i, InNum, 10);
//		strcat(outputFile, InNum);
//		strcat(outputFile, ".raw");
//		
//		pf = fopen(inputFile, "rb");
//		fread(Src_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
//		fclose(pf);
//		pf = NULL;
//
//		Filter.ProcessImage(Dst_data, Src_data, DewarpTable);
//
//		pf = fopen(outputFile, "wb+");
//		fwrite(Dst_data, 1, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2 * sizeof(uint16_t), pf);
//		fclose(pf);
//		pf = NULL;
//		memset(inputFile, 0, 128 * sizeof(char));
//		memset(outputFile, 0, 128 * sizeof(char));
//	}
//	
//	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
//	
//	free(Src_data);
//	Src_data = NULL;
//	free(Dst_data);
//	Dst_data = NULL;
//	free(DewarpTable);
//	DewarpTable = NULL;
//
//	return 1;
//}