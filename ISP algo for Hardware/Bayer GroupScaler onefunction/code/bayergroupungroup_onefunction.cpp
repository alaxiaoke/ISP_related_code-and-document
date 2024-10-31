#include "bayergroupungroup_onefunction.h"


int32_t GROUP::CalTrans(int32_t group_flag, int32_t j, int32_t ws, int32_t bayerscalerw){

	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;
	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;
	
	int32_t trans;
	int left;
	int right;
	int transtmp;

	if (group_flag == 0) {
		transtmp = max(min(min(j, ws - j), overlapsize), 0);
		left = transtmp >> step;
		left = min(left, overlaptablen - 1);
		right = min(left + 1, overlaptablen - 1);
		trans = overlaptable[left] + ((overlaptable[right] - overlaptable[left]) * (transtmp - (left << step)) >> step);
		if (bayerscalerw != (1<<SWPRECISION))
			trans = 0;
	}
	else {
		trans = 0;
	}
	return trans;
}


int32_t GROUP::GetCoorPhaseOne(
	int32_t* wt,                        //FIXME
	int32_t* wto,
	int32_t* wst,
	int32_t widthg,
	int32_t widthin,
	int32_t bayerscalerw,

	int32_t wo1padlen,
	int32_t lutwlogstep,
	int32_t lutwlenleft,
	int32_t lutwlenall,
	int32_t* lutw
){

	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;
	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;

	int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	int32_t distWs = n_sInternalParam.distWs;
	int32_t trans = n_sInternalParam.trans;
	int32_t swinv = n_sInternalParam.swinv;



	int imj;
	int group_flag;
	int ws, j, indexoff, indexoffin;
	imj = n_sInternalParam.rown;
	if (imj < wto[0]) {
		group_flag = wst[0];
		ws = wto[0];
		j = imj;
		indexoff = 0;
		indexoffin = 0;
	}
	else if (imj < wto[1]) {
		group_flag = wst[1];
		ws = wto[1] - wto[0];
		j = imj - wto[0];
		indexoff = wto[0];
		indexoffin = wt[0];
	}
	else {
		group_flag = wst[2];
		ws = widthg - wto[1];
		j = imj - wto[1];
		indexoff = wto[1];
		indexoffin = wt[1];
	}
	int juse,juseoff,indexr;
	int sw;   //3.12    step
	int arrayWs; //12.12
	int index, lutoff;
	int transtmp; //12

	int left;
	int right;
	if (group_flag == 0) {
		sw = bayerscalerw;
		swinv = ((1 << SWPRECISION) << SWINVPRECISION) / sw;        //fixme
		arrayWs = sw * j;
	}
	else {
		if (group_flag == 1) {
				lutoff = 0;
				juse = j+ wo1padlen;
		}
		else {
			lutoff = lutwlenleft;
			juse = j;
		}
		juseoff = (juse>>lutwlogstep);
		index = lutoff + juseoff;
		indexr = index+1;
		if(group_flag == 1){
			index = min(index, lutwlenleft - 1); 
			indexr = min(indexr,lutwlenleft - 1);
		}
		else{
			index = min(index, lutwlenall - 1); 
			indexr = min(indexr,lutwlenall - 1);
		}
		
		sw = (lutw[indexr] - lutw[index])>>lutwlogstep;
		sw = max(sw,1<<SWPRECISION);

		swinv = ((1 << SWPRECISION) << SWINVPRECISION) / sw;
		arrayWs = lutw[index] + (((lutw[indexr]-lutw[index]) * (juse-(juseoff<<lutwlogstep))) >> lutwlogstep);
	}
	arrayWfloor = arrayWs >> SWPRECISION;
	distWs = arrayWs - (arrayWfloor << SWPRECISION);
	trans = CalTrans(group_flag, j, ws, bayerscalerw);

	int32_t wsum = 0;            //4.15
	int32_t imsum = 0;           //17.15
	int32_t tmpj;  //12
	int32_t swinvu; //0.8
	int32_t phaseWjj; //s4.20
	int32_t indx;   //8
	int32_t tmpw;   //1.15 wprecision
	int32_t ww1;    //1.15

	arrayWfloor = indexoffin + arrayWfloor;
	arrayWfloor = max(min(arrayWfloor,widthin-1),0);
	

	n_sInternalParam.arrayWfloor = arrayWfloor;
	n_sInternalParam.distWs = distWs;
	n_sInternalParam.trans = trans;
	n_sInternalParam.swinv = swinv;
	return 1;
}

int32_t GROUP::GetCoorPhaseUngroupOne(
	int32_t* wt,
	int32_t* wto,
	int32_t* wst,
	int32_t widthg,
	int32_t bayerscalerw,
	int32_t lutwlenall,
	int32_t lutwlenleft,
	int32_t ho1padlen,
	int32_t lutwlogstep,
	int32_t* lutw
){

	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;
	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;

	int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	int32_t distWs = n_sInternalParam.distWs;
	int32_t trans = n_sInternalParam.trans;
	int imj = n_sInternalParam.rown;

	int group_flag;
	int ws, j, indexoff, indexoffin;

	if (imj < wt[0]) {
		group_flag = wst[0];
		ws = wto[0];
		j = imj;
		indexoff = 0;
		indexoffin = 0;
	}
	else if (imj < wt[1]) {
		group_flag = wst[1];
		ws = wto[1] - wto[0];
		j = imj - wt[0];
		indexoff = wt[0];
		indexoffin = wto[0];
	}
	else {
		group_flag = wst[2];
		ws = widthg - wto[1];
		j = imj - wt[1];
		indexoff = wt[1];
		indexoffin = wto[1];
	}

	int sw;   //3.12    step
	int64_t arrayWs; //12.12
	int index, lutoff;
	int transtmp; //12
	int left;
	int right;
	int lutlen;

	int64_t leftlut,rightlut;
	int shiftbit = lutwlogstep+SWPRECISION;
	

	if (group_flag == 0) {
		sw = bayerscalerw;
		arrayWs = sw * j;
	}
	else {
		if (group_flag == 1) {
			lutoff = 0;
			lutlen = lutwlenleft;
		}
		else {
			lutoff = lutwlenleft;
			lutlen = lutwlenall-lutwlenleft;
		}
		if(j==0){
			n_sInternalParam.leftlut = 0;
			n_sInternalParam.rightlut =min(n_sInternalParam.leftlut+1, lutlen - 1);
		}

		rightlut = n_sInternalParam.rightlut;                       // FIXME: this
		leftlut = n_sInternalParam.leftlut;
		// look for i
		if((j<<SWPRECISION)>lutw[rightlut+lutoff]){
			leftlut = rightlut;
		}
		rightlut = min(leftlut+1, lutlen - 1);
		if(lutw[rightlut+lutoff]==lutw[leftlut+lutoff])
			arrayWs = (leftlut<<shiftbit);
		else
			arrayWs = (leftlut<<shiftbit) + ((rightlut<<SWPRECISION)-(leftlut<<SWPRECISION))*((j<<SWPRECISION)-lutw[leftlut+lutoff])/((lutw[rightlut+lutoff]-lutw[leftlut+lutoff])>>lutwlogstep);

		if (group_flag ==1){
			arrayWs = max((arrayWs-(ho1padlen<<SWPRECISION)), 0);
		}
		n_sInternalParam.rightlut = rightlut;
		n_sInternalParam.leftlut = leftlut;
	}
	arrayWfloor = arrayWs >> SWPRECISION;
	distWs = arrayWs - (arrayWfloor << SWPRECISION);
	
	trans = CalTrans(group_flag, j, ws, bayerscalerw);


	int32_t wsum = 0;            //4.15
	int32_t imsum = 0;           //17.15
	int32_t tmpj;  //12
	int32_t swinvu; //0.8
	int32_t phaseWjj; //s4.20
	int32_t indx;   //8
	int32_t tmpw;   //1.15 wprecision
	int32_t ww1;    //1.15

	arrayWfloor = indexoffin + arrayWfloor;
	arrayWfloor = max(min(arrayWfloor,widthg-1),0);
	
	n_sInternalParam.arrayWfloor = arrayWfloor;
	n_sInternalParam.distWs = distWs;
	n_sInternalParam.trans = trans;
	return 1;
}

int32_t GROUP::GetLinebuffUngroupOne(uint16_t* Src){
	int32_t* Coor = n_sInternalParam.CoorUn;
	int32_t i,j,index;
	for(i=0; i<LINE_BUFFER_HEIGHT_UN; i++){
		index = Coor[i] * n_sCtrlParam.RegOutputWidth;
		for(j=0; j<n_sCtrlParam.RegOutputWidth;j++){
			pLineBuffUngroup[i][j] = Src[index+j];
		}
	}
	return 1;
}

int32_t GROUP::GetLinebuffOne(uint16_t* Src){
	int32_t* Coor = n_sInternalParam.Coor;
	int32_t i,j,index;
	for(i=0; i<LINE_BUFFER_HEIGHT; i++){
		index = Coor[i] * n_sCtrlParam.RegOutputWidth;
		for(j=0; j<n_sCtrlParam.RegOutputWidth;j++){
			pLineBuff[i][j] = Src[index+j];
		}
	}
	return 1;
}

int32_t GROUP::CalWeightLineIdxOne(int32_t rown, int32_t maxcoor){

	// int32_t tap = TAP;
	// int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	// int32_t distWs = n_sInternalParam.distWs;
	// int32_t trans = n_sInternalParam.trans;


	int32_t tap = TAP;
	int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	int32_t distWs = n_sInternalParam.distWs;
	int32_t trans = n_sInternalParam.trans;
	int32_t swinvu = n_sInternalParam.swinv;

	int32_t* tabley;
	int32_t* Weight;
	int32_t* Coor;

	if(n_sInternalParam.grouporungroup==0){
		tabley = n_sCtrlParam.Regtableygroup;
		Weight = n_sInternalParam.Weight;
		Coor = n_sInternalParam.Coor;
		//rown = n_sInternalParam.rown;
	}
	else{
		tabley = n_sCtrlParam.Regtableyungroup;
		Weight = n_sInternalParam.WeightUn;
		Coor = n_sInternalParam.CoorUn;
	}

	int32_t i;
	int32_t ww1;
	int32_t phaseWjj;
	int32_t indx;
	int32_t tmpw;
	int32_t tmpcoor;

	int LINE_BUFFER_USE;
	if(n_sInternalParam.grouporungroup==0){
		LINE_BUFFER_USE = FILTER_HEIGHT;
		// maxcoor = n_sCtrlParam.RegInputHeight;
	}
	else{
		LINE_BUFFER_USE = LINE_BUFFER_HEIGHT_UN;
	}

	for(i=0; i<LINE_BUFFER_USE; i++){
		tmpcoor =arrayWfloor+i-(LINE_BUFFER_USE>>1)+1;
		Coor[i] = clip(tmpcoor, 0, maxcoor-1);
		if(n_sInternalParam.grouporungroup==0){
			phaseWjj = (((i-(LINE_BUFFER_USE>>1)+1) << SWPRECISION) - distWs) * (swinvu>>1);
		}
		else{
			phaseWjj = (((i-(LINE_BUFFER_USE>>1)+1) << SWPRECISION) - distWs) >>1;
		}

		if (phaseWjj < 0) {
			phaseWjj = -phaseWjj;
		}
		if(n_sInternalParam.grouporungroup==0){
			indx = (phaseWjj << tap) >> (SWPRECISION + SWINVPRECISION);
		}
		else{
			indx = (phaseWjj << tap) >> (SWPRECISION);
		}
		
		
		if (indx >= (1<<(TAP+1))) {
                indx = (1<<(TAP+1))-1;
		}
		if (i == (LINE_BUFFER_USE>>1) || i == (LINE_BUFFER_USE>>1)-1) {
			ww1 = 1 << WPRECISION;
		}
		else {
			ww1 = 0;
		}
		tmpw = (ww1 * trans + tabley[indx] * ((1 << OVERLAPPRECISION) - trans)) >> OVERLAPPRECISION;
		if (rown % 2 != Coor[i] % 2) {
			tmpw = 0;
		}
		Weight[i] = tmpw;
	}
	return 1;
}

int32_t GROUP::VerProcOneLineOne(uint16_t* Dst) {
	int32_t* tmpw;
	int j;
	int32_t imsum;
	int32_t wsum;	
	int32_t imtmp,wtmp;
	int32_t jwidth;
	if(n_sInternalParam.grouporungroup==0){
		jwidth = n_sCtrlParam.RegOutputWidth;
		tmpw = n_sInternalParam.Weight;
	}
	else{
		jwidth = n_sCtrlParam.RegOutputWidth;
		tmpw = n_sInternalParam.WeightUn;
	}
	for(j=0; j<jwidth; j++) {
		imsum = 0;
		wsum = 0;
		for(int ii = 0; ii < FILTER_HEIGHT; ii++) {
			if(n_sInternalParam.grouporungroup==0){
				imtmp = pLineBuff[ii][j];
				wtmp = tmpw[ii];
			}
			else{
				if(ii<LINE_BUFFER_HEIGHT_UN){
					imtmp = pLineBuffUngroup[ii][j];
					wtmp = tmpw[ii];
				}
				else{
					imtmp=0;
					wtmp=0;
				}	
			}		
			imsum = imsum + imtmp * wtmp;
			wsum = wsum + wtmp;
		}
		//Dst[j] = uint16_t(imsum / wsum);	
		//Dst[j] = uint16_t(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
		Dst[j] = (unsigned short)(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
	}
    return 1;
}

int32_t GROUP::HorProcOneLineOne(uint16_t* Dst, uint16_t* Src) {

	int32_t widthin = n_sCtrlParam.RegInputWidth;
	int32_t heightin = n_sCtrlParam.RegInputHeight;
	int32_t widthg = n_sCtrlParam.RegOutputWidth;
	int32_t heightg = n_sCtrlParam.RegOutputHeight;

	int32_t* wt = n_sCtrlParam.Regwt;
	int32_t* ht = n_sCtrlParam.Reght;
	int32_t* wto = n_sCtrlParam.Regwto;
	int32_t* hto = n_sCtrlParam.Reghto;
	int32_t* wst = n_sCtrlParam.Regwst;
	int32_t* hst = n_sCtrlParam.Reghst;
	int32_t* lutw = n_sCtrlParam.Reglutw;                      //12.12

	int32_t lutwlenall = n_sCtrlParam.Reglutwlenall;
	int32_t lutwlenleft = n_sCtrlParam.Reglutwlenleft;
	int32_t wo1padlen = n_sCtrlParam.Regwo1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Reglutwlogstep;

	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;                      //log(step)
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;

	int32_t tap = TAP;                        //log(tap)


	int32_t maxcoor;
	int32_t bayerscalerw;
	int32_t* tabley;

	if(n_sInternalParam.grouporungroup==0){

		bayerscalerw = n_sCtrlParam.Regbayerscalerw;
		tabley = n_sCtrlParam.Regtableygroup;
		maxcoor = n_sCtrlParam.RegInputWidth;


		int trans; //1.8
		int arrayWfloor; //12
		int distWs;      //.12
		int swinv; //1.8    phase


		int imj;
		int group_flag;
		int ws, j, indexoff, indexoffin;
		int32_t imsum;
		int32_t wsum;	
		for (imj = 0; imj < widthg; imj++) {
			
			n_sInternalParam.rown = imj;
			GetCoorPhaseOne(wt, wto, wst, widthg, widthin, bayerscalerw, wo1padlen, lutwlogstep,lutwlenleft,lutwlenall,lutw);
			CalWeightLineIdxOne(imj,maxcoor);
			int32_t* tmpw = n_sInternalParam.Weight;
			int32_t* Coor = n_sInternalParam.Coor;

			int j;
			// int32_t imsum;
			// int32_t wsum;	

			imsum = 0;
			wsum = 0;
			for(int ii = 0; ii < FILTER_WIDTH; ii++) {
				imsum = imsum + Src[Coor[ii]] * tmpw[ii];
				wsum = wsum + tmpw[ii];
			}
			//Dst[imj] = uint16_t(imsum / wsum);
			// Dst[imj] = uint16_t(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
			Dst[imj] = (unsigned short)(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
		}
	}
	else{
		bayerscalerw = (1<<SWPRECISION);
		maxcoor = n_sCtrlParam.RegOutputWidth;
		tabley = n_sCtrlParam.Regtableyungroup;
		int32_t imsum;
		int32_t wsum;	
		int imj;
		int group_flag;
		int ws, j, indexoff, indexoffin;
		for (imj = 0; imj < widthin; imj++) {
			n_sInternalParam.rown = imj;
			GetCoorPhaseUngroupOne(wt, wto, wst, widthg, bayerscalerw, lutwlenall, lutwlenleft, wo1padlen, lutwlogstep,lutw);
			CalWeightLineIdxOne(imj,maxcoor);
			int32_t* tmpw = n_sInternalParam.WeightUn;
			int32_t* Coor = n_sInternalParam.CoorUn;
			int j;
			imsum = 0;
			wsum = 0;
			for(int ii = 0; ii < LINE_BUFFER_HEIGHT_UN; ii++) {
				imsum = imsum + Src[Coor[ii]] * tmpw[ii];
				wsum = wsum + tmpw[ii];
			}
			//Dst[imj] = uint16_t(imsum / wsum);	
			// Dst[imj] = uint16_t(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
			Dst[imj] = (unsigned short)(clip(((imsum<<1)+wsum)/(wsum<<1),0,MAX_DATA_VALUE));
		}
	}
	return 1;
}

int32_t GROUP::VerProcOne(uint16_t* Dst_Data, uint16_t* Src_Data){
	int32_t maxcoor;

	int32_t* wt = n_sCtrlParam.Reght;                        
	int32_t* wto = n_sCtrlParam.Reghto;
	int32_t* wst = n_sCtrlParam.Reghst;
	int32_t widthg = n_sCtrlParam.RegOutputHeight;
	int32_t widthin = n_sCtrlParam.RegInputHeight;

	int32_t wo1padlen = n_sCtrlParam.Regho1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Regluthlogstep;
	int32_t lutwlenleft = n_sCtrlParam.Regluthlenleft;
	int32_t lutwlenall = n_sCtrlParam.Regluthlenall;
	int32_t* lutw = n_sCtrlParam.Regluth;                     //12.12

	if(n_sInternalParam.grouporungroup==0){
		int32_t bayerscalerw = n_sCtrlParam.Regbayerscalerh;
		maxcoor = n_sCtrlParam.RegInputHeight;
		pLineBuff[0] = new uint16_t[LINE_BUFFER_HEIGHT * LINE_BUFFER_WIDTH];
	
		for (int32_t i = 1; i < LINE_BUFFER_HEIGHT; i++) {
			pLineBuff[i] = pLineBuff[i - 1] + LINE_BUFFER_WIDTH;
		}
		if (!pLineBuff[0])
		{
			printf("GROUP::pLineBuff malloc fail\n");
		}


		int32_t i;
		int32_t tmpw[FILTER_WIDTH];
		uint16_t* Dst = Dst_Data;

		for(i=0; i<n_sCtrlParam.RegOutputHeight; i++) {	
			n_sInternalParam.rown = i;
			GetCoorPhaseOne(wt, wto, wst, widthg, widthin, bayerscalerw, wo1padlen, lutwlogstep,lutwlenleft,lutwlenall,lutw);
			CalWeightLineIdxOne(i, maxcoor);
			GetLinebuffOne(Src_Data);
			VerProcOneLineOne(Dst);	
			Dst += n_sCtrlParam.RegOutputWidth;	
		}	

		if(pLineBuff[0]){
			delete pLineBuff[0];
		}

	}
	else{
		int32_t i;
		int32_t tmpw[LINE_BUFFER_HEIGHT_UN];
		uint16_t* Dst = Dst_Data;
		int32_t bayerscalerw = (1<<SWPRECISION);             //FIXME : this
		maxcoor = n_sCtrlParam.RegOutputHeight;

		pLineBuffUngroup[0] = new uint16_t[LINE_BUFFER_HEIGHT_UN * LINE_BUFFER_WIDTH];
		for (int32_t i = 1; i < LINE_BUFFER_HEIGHT_UN; i++) {
			pLineBuffUngroup[i] = pLineBuffUngroup[i - 1] + LINE_BUFFER_WIDTH;
		}
		if (!pLineBuffUngroup[0])
		{
			printf("GROUP::pLineBuff malloc fail\n");
		}

		for(i=0; i<n_sCtrlParam.RegInputHeight; i++) {	
			n_sInternalParam.rown = i;
			
			GetCoorPhaseUngroupOne(wt, wto, wst, widthg, bayerscalerw, lutwlenall, lutwlenleft, wo1padlen, lutwlogstep,lutw);
			CalWeightLineIdxOne(i, maxcoor);
			GetLinebuffUngroupOne(Src_Data);
			VerProcOneLineOne(Dst);
			Dst += n_sCtrlParam.RegOutputWidth;	
		}

		if(pLineBuffUngroup[0]){
			delete pLineBuffUngroup[0];
		}
		
	}

	return 1;	
}

int32_t GROUP::HorProcOne(uint16_t* Dst_Data, uint16_t* Src_Data){
	uint16_t* Src = Src_Data;
	uint16_t* Dst = Dst_Data;
	
	int RowIdx;
	if(n_sInternalParam.grouporungroup==0){		
		for (RowIdx = 0; RowIdx < n_sCtrlParam.RegInputHeight; RowIdx++) {
			HorProcOneLineOne(Dst, Src);
			Src += n_sCtrlParam.RegInputWidth;
			Dst += n_sCtrlParam.RegOutputWidth;
		}
	}
	else{
		for (RowIdx = 0; RowIdx < n_sCtrlParam.RegInputHeight; RowIdx++) {
			HorProcOneLineOne(Dst, Src);
			Src += n_sCtrlParam.RegOutputWidth;
			Dst += n_sCtrlParam.RegInputWidth;
		}
	}
	return 1;
}

int32_t GROUP::GroupUngroup(uint16_t* Dst_Data, uint16_t* Src_Data, int mode){
	//mode=0:group, mode=1: ungroup
	n_sInternalParam.grouporungroup = mode;
	// uint16_t* HorOut = (uint16_t*)malloc(sizeof(uint16_t) * n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegInputHeight);
	if(n_sInternalParam.grouporungroup ==0){
		if(n_sCtrlParam.RegEnableGroup){
			uint16_t* HorOut = (uint16_t*)malloc(sizeof(uint16_t) * n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegInputHeight);
			if(!HorOut){
				printf("GROUP::HorOut malloc fail\n");
			}
			HorProcOne(HorOut, Src_Data);
			VerProcOne(Dst_Data, HorOut);
			if(HorOut)
				free(HorOut);
		}
		else{
			memcpy(Dst_Data, Src_Data, n_sCtrlParam.RegInputWidth * n_sCtrlParam.RegInputHeight * sizeof(uint16_t));
		}
	}
	else{
		if (n_sCtrlParam.RegEnableUngroup){
			uint16_t* HorOut2 = (uint16_t*)malloc(sizeof(uint16_t) * n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegInputHeight);
			if(!HorOut2)
			{
				printf("GROUP::HorOut malloc fail\n");
			}

			VerProcOne(HorOut2, Src_Data);
			HorProcOne(Dst_Data, HorOut2);
			 if(HorOut2)
			 	free(HorOut2);
		}
		else{
			memcpy(Dst_Data, Src_Data, n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegOutputHeight * sizeof(uint16_t));
		}
	}
	//  if(HorOut)
	//   	free(HorOut);
	return 1;
}