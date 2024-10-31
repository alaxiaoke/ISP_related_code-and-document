//#include "pch.h"
#include "bayergroupungroup.h"
#include "stdint.h"
// using namespace winrt;
// using namespace Windows::Foundation;

int main()
{
    GROUP Group;

    // Group.n_sCtrlParam.RegInputWidth = 2328;
    // Group.n_sCtrlParam.RegInputHeight = 1748;
    // Group.n_sCtrlParam.RegOutputWidth = 2328;

	char parametername[100]={0};
	char outputname[100]={0};

    int32_t* parameter = NULL;
    int32_t parameter_size = 870; //868;   //FIXME
    FILE* pf;
	for(int fnum=0;fnum<90; fnum++){
		parameter = (int32_t*)malloc(parameter_size * sizeof(int32_t));
		sprintf(parametername, "D://Data//result//AI2D//parameterstatictestnew2//%dparameterscaler.raw",fnum);
		pf = fopen(parametername, "rb");
		// pf = fopen("D://Data//result//AI2D//parameterstatictestnew2//0parameterscaler.raw", "rb");
		if (pf == NULL)
		{
			printf("Read parameter error!\n");
			return 0;
		}
		else
		{
			fread(parameter, 1, parameter_size * sizeof(int32_t), pf);
		}
		fclose(pf);


		int32_t* parameter2 = NULL;
		int32_t parameter2_size = 147; // 83;   //FIXME
		//FILE* pf;
		parameter2 = (int32_t*)malloc(parameter2_size * sizeof(int32_t));
		pf = fopen("D://Project//goupungroup//goupungroup//parameter2.raw", "rb");
		if (pf == NULL)
		{
			printf("Read parameter2 error!\n");
			return 0;
		}
		else
		{
			fread(parameter2, 1, parameter2_size * sizeof(int32_t), pf);
		}
		fclose(pf);

		Group.n_sCtrlParam.RegEnableGroup = 1;
		Group.n_sCtrlParam.RegEnableUngroup = 1;

		Group.n_sCtrlParam.RegInputWidth = 2328;
		Group.n_sCtrlParam.RegInputHeight = 1748;
		Group.n_sCtrlParam.RegOutputWidth = parameter[0];
		Group.n_sCtrlParam.RegOutputHeight = parameter[1];

		// memcpy(Group.n_sCtrlParam.wt, new int32_t[2]{1, 1}, sizeof(int32_t)*2); 
		Group.n_sCtrlParam.Regwt[0] = parameter[2];
		Group.n_sCtrlParam.Regwt[1] = parameter[3];
		Group.n_sCtrlParam.Reght[0] = parameter[4];
		Group.n_sCtrlParam.Reght[1] = parameter[5];
		Group.n_sCtrlParam.Regwto[0] = parameter[6];
		Group.n_sCtrlParam.Regwto[1] = parameter[7];
		Group.n_sCtrlParam.Reghto[0] = parameter[8];
		Group.n_sCtrlParam.Reghto[1] = parameter[9];
		Group.n_sCtrlParam.Regwst[0] = parameter[10];
		Group.n_sCtrlParam.Regwst[1] = parameter[11];
		Group.n_sCtrlParam.Regwst[2] = parameter[12];
		Group.n_sCtrlParam.Reghst[0] = parameter[13];
		Group.n_sCtrlParam.Reghst[1] = parameter[14];
		Group.n_sCtrlParam.Reghst[2] = parameter[15];
		Group.n_sCtrlParam.Reglutwlenall = parameter[16];
		Group.n_sCtrlParam.Regluthlenall = parameter[17];
		Group.n_sCtrlParam.Reglutwlenleft = parameter[18];
		Group.n_sCtrlParam.Regluthlenleft = parameter[19];
		printf("Reglutwlenleft=%d\n",parameter[18]);
		printf("Regluthlenleft=%d\n",parameter[19]);

		Group.n_sCtrlParam.Reglutw = (int32_t*)malloc(sizeof(int32_t) * Group.n_sCtrlParam.Reglutwlenall);      //FIXME
		Group.n_sCtrlParam.Regluth = (int32_t*)malloc(sizeof(int32_t) * Group.n_sCtrlParam.Regluthlenall);     //FIXME

		for(int i=0; i<Group.n_sCtrlParam.Reglutwlenall; i++){
			//printf("parameter2[i+20]=%d\n", parameter2[i+20]);
			Group.n_sCtrlParam.Reglutw[i] = parameter[i + 20];             
		}
		for(int i=0; i<Group.n_sCtrlParam.Regluthlenall; i++){
			Group.n_sCtrlParam.Regluth[i] = parameter[i + 20+Group.n_sCtrlParam.Reglutwlenall];
			if(i<3){
				int tmp =(Group.n_sCtrlParam.Regluth[i]>>12);
				int phase = Group.n_sCtrlParam.Regluth[i] - (tmp<<12);
				// printf("luth[%d]=%d\n",i, tmp);
				// printf("phase[%d]=%d\n",i, phase);
				// printf("luth[%d]=%d\n",i, (Group.n_sCtrlParam.Regluth[i]>>12));
			}
			
		}

		Group.n_sCtrlParam.Regbayerscalerw = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall];
		Group.n_sCtrlParam.Regbayerscalerh = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall+1];
		Group.n_sCtrlParam.Regwo1padlen = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall+2];                 //new
		Group.n_sCtrlParam.Reglutwlogstep = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall+3];               //new
		Group.n_sCtrlParam.Regho1padlen = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall+4];                 //new
		Group.n_sCtrlParam.Regluthlogstep = parameter[Group.n_sCtrlParam.Regluthlenall + 20+Group.n_sCtrlParam.Reglutwlenall+5];   


		// printf("Group.n_sCtrlParam.Reglutwlenall=%d\n",Group.n_sCtrlParam.Reglutwlenall);  
		// printf("Group.n_sCtrlParam.Regluthlenall=%d\n",Group.n_sCtrlParam.Regluthlenall); 
		// printf("Group.n_sCtrlParam.Regluthlenleft=%d\n",Group.n_sCtrlParam.Regluthlenleft);
		// printf("Group.n_sCtrlParam.Regwo1padlen=%d\n", Group.n_sCtrlParam.Regwo1padlen);     
		// printf("Group.n_sCtrlParam.Reglutwlogstep=%d\n", Group.n_sCtrlParam.Reglutwlogstep); 
		// printf("Group.n_sCtrlParam.Regho1padlen=%d\n", Group.n_sCtrlParam.Regho1padlen); 
		// printf("Group.n_sCtrlParam.Regluthlogstep=%d\n", Group.n_sCtrlParam.Regluthlogstep);     

		// printf("bayerscalerh=%d\n", Group.n_sCtrlParam.Regbayerscalerh);
		// printf("bayerscalerw=%d\n", Group.n_sCtrlParam.Regbayerscalerw);
		// Group.n_sCtrlParam.bayerscalerw=1<<SWPRECISION;
		// Group.n_sCtrlParam.bayerscalerh=1<<SWPRECISION;
		// Group.n_sCtrlParam.Regbayerscalerwun=1<<SWPRECISION;
		// Group.n_sCtrlParam.Regbayerscalerhun=1<<SWPRECISION;
		


		// Group.n_sCtrlParam.tableygroup;
		Group.n_sCtrlParam.Regoverlapsize = parameter2[0];
		Group.n_sCtrlParam.Regoverlaplogstep = parameter2[1];                      //log(step)
		Group.n_sCtrlParam.Regoverlaptablen = parameter2[2];
		for(int i=0; i<Group.n_sCtrlParam.Regoverlaptablen; i++){
			Group.n_sCtrlParam.Regoverlaptable[i] = parameter2[3+i];
		}
		for(int i=0; i<(1<<TAP)*2; i++){
			Group.n_sCtrlParam.Regtableygroup[i] =parameter2[3+parameter2[2]+i];
			// printf("Group.n_sCtrlParam.tableygroup[%d]=%d\n",i,Group.n_sCtrlParam.Regtableygroup[i]);
		}
		for(int i=0; i<(1<<TAP)*2; i++){
			Group.n_sCtrlParam.Regtableyungroup[i] =parameter2[3+parameter2[2]+(1<<TAP)*2+i];
			// printf("Group.n_sCtrlParam.tableyungroup[%d]=%d\n",i,Group.n_sCtrlParam.Regtableyungroup[i]);
		}
		

		uint16_t* data_src = NULL;
		uint16_t* data_dst = NULL;
		uint16_t* data_out = NULL;
		//FILE* pf;
		data_src = (uint16_t*)malloc(Group.n_sCtrlParam.RegInputWidth * Group.n_sCtrlParam.RegInputHeight * sizeof(uint16_t));
		pf = fopen("D://Data//rawdata//12800//static//20221201-144418-2328-1748-RGGB.raw", "rb");
		if (pf == NULL)
		{
			printf("Read image error!\n");
			return 0;
		}
		else
		{
			fread(data_src, 1, Group.n_sCtrlParam.RegInputWidth * Group.n_sCtrlParam.RegInputHeight * sizeof(uint16_t), pf);
		}
		fclose(pf);


		data_dst = (uint16_t*)malloc(Group.n_sCtrlParam.RegOutputWidth * Group.n_sCtrlParam.RegOutputHeight * sizeof(uint16_t));
		Group.Group(data_dst, data_src);

		pf = fopen("D://Data//result//GroupCmodel//group.raw", "wb+");
		fwrite(data_dst, 1, Group.n_sCtrlParam.RegOutputWidth * Group.n_sCtrlParam.RegOutputHeight * sizeof(uint16_t), pf);
		fclose(pf);
		pf = NULL;
		
		data_out = (uint16_t*)malloc(Group.n_sCtrlParam.RegInputWidth * Group.n_sCtrlParam.RegInputHeight * sizeof(uint16_t));
		if(Group.n_sCtrlParam.Regbayerscalerw==1<<SWPRECISION && Group.n_sCtrlParam.Regbayerscalerh==1<<SWPRECISION){
			Group.Ungroup(data_out, data_dst);
		}
		sprintf(outputname, "D://Data//result//GroupCmodel//%dgroup.raw",fnum);
		pf = fopen(outputname, "wb+");
		fwrite(data_out, 1, Group.n_sCtrlParam.RegInputWidth * Group.n_sCtrlParam.RegInputHeight * sizeof(uint16_t), pf);
		fclose(pf);
		printf("Ungroup is done\n");
		// pf = fopen("D://Data//result//GroupCmodel//ungroup.raw", "wb+");
		// fwrite(data_src, 1, Group.n_sCtrlParam.RegInputWidth * Group.n_sCtrlParam.RegInputHeight * sizeof(uint16_t), pf);
		// fclose(pf);
		// pf = NULL;


		free(data_dst);
		free(data_src);
		free(data_out);
		free(Group.n_sCtrlParam.Reglutw);
		free(Group.n_sCtrlParam.Regluth);
		free(parameter2);
		free(parameter);
	}

    //init_apartment();
    //Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello,22 \n");
	return 1;
}
