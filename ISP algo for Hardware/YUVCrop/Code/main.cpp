#include"CYuvCrop.h"

//test sigle
int main(int argc, char** argv)
//argv[1]: input file (yuv422 10bit) 
//argv[2]: Output file (yuv422 10bit) 
//argv[3]: Settingfile 
{
	CYuvCrop Filter;
	//char Filename[128] = "YuvCropSetting.txt";
	char* Filename = argv[3];
	//Filter.SaveSettingFile(Filename);
	Filter.LoadSettingFile(Filename);


	uint16_t* Src_data = NULL;
	uint16_t* Dst_data = NULL;

	FILE* pf;

	Src_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegInputWidth * Filter.m_sCtrlParam.RegInputHeight * 2 * sizeof(uint16_t));
	
	pf = fopen(argv[1], "rb");
	fread(Src_data, 1, Filter.m_sCtrlParam.RegInputWidth * Filter.m_sCtrlParam.RegInputHeight * 2 * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;

	Dst_data = (uint16_t*)malloc(Filter.m_sCtrlParam.RegCropWidth * Filter.m_sCtrlParam.RegCropHeight * 2 * sizeof(uint16_t));
	Filter.ProcessImage(Dst_data, Src_data);
	//memcpy(Dst_data, Src_data, Filter.m_sCtrlParam.RegFullWidth * Filter.m_sCtrlParam.RegFullHeight * 2*sizeof(uint16_t));
	pf = fopen(argv[2], "wb+");
	fwrite(Dst_data, 1, Filter.m_sCtrlParam.RegCropWidth * Filter.m_sCtrlParam.RegCropHeight * 2 * sizeof(uint16_t), pf);
	fclose(pf);
	pf = NULL;

	free(Src_data);
	Src_data = NULL;
	free(Dst_data);
	Dst_data = NULL;

	return 1;
}