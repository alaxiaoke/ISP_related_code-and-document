#include "bayergroupungroup.h"


int32_t GROUP::HorProcOneLine(uint16_t* Dst, uint16_t* Src) {
	
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
	int32_t wo1padlen = n_sCtrlParam.Regwo1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Reglutwlogstep;
	int32_t lutwlenleft = n_sCtrlParam.Reglutwlenleft;
	int32_t lutwlenall = n_sCtrlParam.Reglutwlenall;

	int32_t bayerscalerw = n_sCtrlParam.Regbayerscalerw;
	int32_t tap = TAP;                        //log(tap)
	int32_t* tabley = n_sCtrlParam.Regtableygroup;

	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;                      //log(step)
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;

	int imj;
	int group_flag;
	int ws, j, indexoff, indexoffin;
	for (imj = 0; imj < widthg; imj++) {
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
		int juse,juseoff;
		int indexr;
		int sw;   //3.12    step
		int swinv; //1.8    phase
		int arrayWs; //12.12
		int arrayWfloor; //12
		int distWs;      //.12
		int index, lutoff;
		int transtmp; //12
		int left, right;
		int trans; //1.8
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
		//overlappresion = 8
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
		for (int jj = 1 - (FILTER_WIDTH >> 1); jj < (FILTER_WIDTH >> 1) + 1; jj++) {
			tmpj = min(max(arrayWfloor + jj, 0), widthin - 1);
			swinvu = swinv >> 1;
			phaseWjj = ((jj << SWPRECISION) - distWs) * swinvu;
			if (phaseWjj < 0) {
				phaseWjj = -phaseWjj;
			}
			indx = (phaseWjj << TAP) >> (SWPRECISION + SWINVPRECISION);
			if (indx >= (1<<(TAP+1))) {
                indx = (1<<(TAP+1))-1;
				}
			tmpw = tabley[indx];
			if (jj == 0 || jj == 1) {
				ww1 = 1 << WPRECISION;
			}
			else {
				ww1 = 0;
			}	
			tmpw = (ww1 * trans + tmpw * ((1 << OVERLAPPRECISION) - trans)) >> OVERLAPPRECISION;
			if (j % 2 != tmpj % 2) {
				tmpw = 0;
			}
			imsum = imsum + Src[tmpj] * tmpw;
			wsum = wsum + tmpw;
		}
		Dst[indexoff + j] = uint16_t(imsum / wsum);
	}
	return 1;
}

int32_t GROUP::HorProc(uint16_t* Dst_Data, uint16_t* Src_Data) {
	uint16_t* Src = Src_Data;
	uint16_t* Dst = Dst_Data;
	int RowIdx;
	for (RowIdx = 0; RowIdx < n_sCtrlParam.RegInputHeight; RowIdx++) {
		HorProcOneLine(Dst, Src);
		Src += n_sCtrlParam.RegInputWidth;
		Dst += n_sCtrlParam.RegOutputWidth;
	}
	return 1;
}

int32_t GROUP::GetCoorPhase(){

	int32_t* wt = n_sCtrlParam.Reght;                        //FIXME
	int32_t* wto = n_sCtrlParam.Reghto;
	int32_t* wst = n_sCtrlParam.Reghst;
	int32_t widthg = n_sCtrlParam.RegOutputHeight;
	int32_t widthin = n_sCtrlParam.RegInputHeight;
	int32_t bayerscalerw = n_sCtrlParam.Regbayerscalerh;

	int32_t wo1padlen = n_sCtrlParam.Regho1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Regluthlogstep;
	int32_t lutwlenleft = n_sCtrlParam.Regluthlenleft;
	int32_t lutwlenall = n_sCtrlParam.Regluthlenall;
	int32_t* lutw = n_sCtrlParam.Regluth;                     //1.12


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
	//overlappresion = 8
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

int32_t GROUP::CalWeightLineIdx(){
	int32_t* tabley = n_sCtrlParam.Regtableygroup;
	int32_t tap = TAP;

	int32_t* Weight = n_sInternalParam.Weight;
	int32_t* Coor = n_sInternalParam.Coor;

	int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	int32_t distWs = n_sInternalParam.distWs;
	int32_t trans = n_sInternalParam.trans;
	int32_t swinvu = n_sInternalParam.swinv;
	int32_t rown = n_sInternalParam.rown;


	int32_t i;
	int32_t ww1;
	int32_t phaseWjj;
	int32_t indx;
	int32_t tmpw;
	int32_t tmpcoor;
	for(i=0; i<FILTER_HEIGHT; i++){
		tmpcoor =arrayWfloor+i-(FILTER_HEIGHT>>1)+1;
		Coor[i] = clip(tmpcoor, 0, n_sCtrlParam.RegInputHeight-1);
		phaseWjj = (((i-(FILTER_HEIGHT>>1)+1) << SWPRECISION) - distWs) * (swinvu>>1);
		if (phaseWjj < 0) {
			phaseWjj = -phaseWjj;
		}
		indx = (phaseWjj << tap) >> (SWPRECISION + SWINVPRECISION);
		if (indx >= (1<<(TAP+1))) {
                indx = (1<<(TAP+1))-1;
				}
		if (i == (FILTER_HEIGHT>>1) || i == (FILTER_HEIGHT>>1)-1) {
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

int32_t GROUP::GetLinebuff(uint16_t* Src){
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

int32_t GROUP::VerProcOneLine(uint16_t* Dst) {
	int32_t* tmpw = n_sInternalParam.Weight;
	int j;
	int32_t imsum;
	int32_t wsum;	
	for(j=0; j<n_sCtrlParam.RegOutputWidth; j++) {
		imsum = 0;
		wsum = 0;
		for(int ii = 0; ii < FILTER_HEIGHT; ii++) {
			imsum = imsum + pLineBuff[ii][j] * tmpw[ii];
			wsum = wsum + tmpw[ii];
		}
		Dst[j] = uint16_t(imsum / wsum);	
	}
    return 1;
}

int32_t GROUP::VerProc(uint16_t* Dst_Data, uint16_t* Src_Data){

	int32_t i;
	int32_t tmpw[FILTER_WIDTH];
	uint16_t* Dst = Dst_Data;

	pLineBuff[0] = new uint16_t[LINE_BUFFER_HEIGHT * LINE_BUFFER_WIDTH];
	
	for (int32_t i = 1; i < LINE_BUFFER_HEIGHT; i++) {
		pLineBuff[i] = pLineBuff[i - 1] + LINE_BUFFER_WIDTH;
	}
	if (!pLineBuff[0])
	{
		printf("GROUP::pLineBuff malloc fail\n");
	}


	for(i=0; i<n_sCtrlParam.RegOutputHeight; i++) {	
		n_sInternalParam.rown = i;
		GROUP::GetCoorPhase();
		GROUP::CalWeightLineIdx();
		GROUP::GetLinebuff(Src_Data);
		GROUP::VerProcOneLine(Dst);
		Dst += n_sCtrlParam.RegOutputWidth;	
	}
	if(pLineBuff[0]){
		delete pLineBuff[0];
	}
	return 1;
}

int32_t GROUP::Group(uint16_t* Dst_Data, uint16_t* Src_Data){
	if(n_sCtrlParam.RegEnableGroup){
		uint16_t* HorOut = (uint16_t*)malloc(sizeof(uint16_t) * n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegInputHeight);
		if(!HorOut){
			printf("GROUP::HorOut malloc fail\n");
		}

		HorProc(HorOut, Src_Data);
		VerProc(Dst_Data, HorOut);
		if(HorOut)
			free(HorOut);
	}
	else{
		memcpy(Dst_Data, Src_Data, n_sCtrlParam.RegInputWidth * n_sCtrlParam.RegInputHeight * sizeof(uint16_t));
	}
	return 1;
}


int32_t GROUP::GetCoorPhaseUngroup(
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

	//overlappresion = 8
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

int32_t GROUP::CalWeightLineIdxUngroup(int32_t rown, int32_t maxcoor){

	int32_t* tabley = n_sCtrlParam.Regtableyungroup;
	int32_t tap = TAP;

	int32_t* Weight = n_sInternalParam.WeightUn;
	int32_t* Coor = n_sInternalParam.CoorUn;
	int32_t arrayWfloor = n_sInternalParam.arrayWfloor;
	int32_t distWs = n_sInternalParam.distWs;
	int32_t trans = n_sInternalParam.trans;


	int32_t i;
	int32_t ww1;
	int32_t phaseWjj;
	int32_t indx;
	int32_t tmpw;
	int32_t tmpcoor;
	for(i=0; i<LINE_BUFFER_HEIGHT_UN; i++){
		tmpcoor =arrayWfloor+i-(LINE_BUFFER_HEIGHT_UN>>1)+1;
		Coor[i] = clip(tmpcoor, 0, maxcoor-1);
		phaseWjj = (((i-(LINE_BUFFER_HEIGHT_UN>>1)+1) << SWPRECISION) - distWs) >>1;
		if (phaseWjj < 0) {
			phaseWjj = -phaseWjj;
		}
		indx = (phaseWjj << tap) >> (SWPRECISION);
		if (indx >= (1<<(TAP+1))) {
                indx = (1<<(TAP+1))-1;
		}
		if (i == (LINE_BUFFER_HEIGHT_UN>>1) || i == (LINE_BUFFER_HEIGHT_UN>>1)-1) {
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

int32_t GROUP::GetLinebuffUngroup(uint16_t* Src){
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

int32_t GROUP::VerProcOneLineUngroup(uint16_t* Dst) {
	int32_t* tmpw = n_sInternalParam.WeightUn;
	int j;
	int32_t imsum;
	int32_t wsum;	
	for(j=0; j<n_sCtrlParam.RegInputWidth; j++) {
		imsum = 0;
		wsum = 0;
		for(int ii = 0; ii < LINE_BUFFER_HEIGHT_UN; ii++) {
			imsum = imsum + pLineBuffUngroup[ii][j] * tmpw[ii];
			wsum = wsum + tmpw[ii];
		}
		Dst[j] = uint16_t(imsum / wsum);	
	}
    return 1;
}

int32_t GROUP::VerProcUngroup(uint16_t* Dst_Data, uint16_t* Src_Data){

	int32_t i;
	int32_t tmpw[LINE_BUFFER_HEIGHT_UN];
	uint16_t* Dst = Dst_Data;

	int32_t* wt = n_sCtrlParam.Reght;                        //FIXME
	int32_t* wto = n_sCtrlParam.Reghto;
	int32_t* wst = n_sCtrlParam.Reghst;
	int32_t widthg = n_sCtrlParam.RegOutputHeight;
	int32_t bayerscalerw = (1<<SWPRECISION);             //FIXME : this

	int32_t lutwlenall = n_sCtrlParam.Regluthlenall;
	int32_t lutwlenleft = n_sCtrlParam.Regluthlenleft;
	int32_t ho1padlen = n_sCtrlParam.Regho1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Regluthlogstep;
	int32_t* lutw = n_sCtrlParam.Regluth;                     //12.12

	int32_t maxcoor = n_sCtrlParam.RegOutputHeight;

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
		
		GROUP::GetCoorPhaseUngroup(wt, wto, wst, widthg, bayerscalerw, lutwlenall, lutwlenleft, ho1padlen, lutwlogstep,lutw);
		GROUP::CalWeightLineIdxUngroup(i,maxcoor);

		GROUP::GetLinebuffUngroup(Src_Data);
		GROUP::VerProcOneLineUngroup(Dst);
		Dst += n_sCtrlParam.RegOutputWidth;	
	}
	if(pLineBuffUngroup[0]){
		delete pLineBuffUngroup[0];
	}
	return 1;
}

int32_t GROUP::HorProcOneLineUngroup(uint16_t* Dst, uint16_t* Src) {
	
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
	int32_t* lutw = n_sCtrlParam.Reglutw;                      //1.12

	int32_t lutwlenall = n_sCtrlParam.Reglutwlenall;
	int32_t lutwlenleft = n_sCtrlParam.Reglutwlenleft;
	int32_t ho1padlen = n_sCtrlParam.Regwo1padlen;
	int32_t lutwlogstep = n_sCtrlParam.Reglutwlogstep;
	int32_t bayerscalerw = (1<<SWPRECISION);
	int32_t maxcoor = n_sCtrlParam.RegOutputWidth;


	int32_t tap = TAP;                        //log(tap)
	int32_t* tabley = n_sCtrlParam.Regtableyungroup;

	int32_t overlapsize = n_sCtrlParam.Regoverlapsize;
	int32_t step = n_sCtrlParam.Regoverlaplogstep;                      //log(step)
	int32_t overlaptablen = n_sCtrlParam.Regoverlaptablen;
	int32_t* overlaptable = n_sCtrlParam.Regoverlaptable;

	int imj;
	int group_flag;
	int ws, j, indexoff, indexoffin;
	for (imj = 0; imj < widthin; imj++) {
		n_sInternalParam.rown = imj;
		GROUP::GetCoorPhaseUngroup(wt, wto, wst, widthg, bayerscalerw, lutwlenall, lutwlenleft, ho1padlen, lutwlogstep,lutw);
		GROUP::CalWeightLineIdxUngroup(imj,maxcoor);
		int32_t* tmpw = n_sInternalParam.WeightUn;
		int32_t* Coor = n_sInternalParam.CoorUn;
		int j;
		int32_t imsum;
		int32_t wsum;	

		imsum = 0;
		wsum = 0;
		for(int ii = 0; ii < LINE_BUFFER_HEIGHT_UN; ii++) {
			imsum = imsum + Src[Coor[ii]] * tmpw[ii];
			wsum = wsum + tmpw[ii];
		}
		Dst[imj] = uint16_t(imsum / wsum);	
	}
	return 1;
}

int32_t GROUP::HorProcUngroup(uint16_t* Dst_Data, uint16_t* Src_Data) {
	uint16_t* Src = Src_Data;
	uint16_t* Dst = Dst_Data;
	int RowIdx;
	for (RowIdx = 0; RowIdx < n_sCtrlParam.RegInputHeight; RowIdx++) {
		HorProcOneLineUngroup(Dst, Src);
		Src += n_sCtrlParam.RegOutputWidth;
		Dst += n_sCtrlParam.RegInputWidth;
	}
	return 1;
}

int32_t GROUP::Ungroup(uint16_t* Dst_Data, uint16_t* Src_Data){
	if (n_sCtrlParam.RegEnableUngroup){
		uint16_t* HorOut = (uint16_t*)malloc(sizeof(uint16_t) * n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegInputHeight);
		if(!HorOut)
		{
			printf("GROUP::HorOut malloc fail\n");
		}

		VerProcUngroup(HorOut, Src_Data);
		HorProcUngroup(Dst_Data, HorOut);
	}
	else{
		memcpy(Dst_Data, Src_Data, n_sCtrlParam.RegOutputWidth * n_sCtrlParam.RegOutputHeight * sizeof(uint16_t));
	}


	return 1;
}