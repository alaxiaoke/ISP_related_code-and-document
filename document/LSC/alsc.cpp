#define _CRT_SECURE_NO_DEPRECATE
#include "alsc.h"


bool SaveAlscTab(const char* savefilename, int32_t* lsctabtoisp, int Height, int Width, int mode_int) {
	FILE* pf;
	if (mode_int == 1) {
		pf = fopen(savefilename, "wb+");
		for (int i = 0; i < Height; i++)
		{
			int index = i * Width;
			for (int j = 0; j < Width; j++)
			{
				fprintf(pf, "%d\n", lsctabtoisp[index + j]);
			}
		}
		fclose(pf);
		pf = NULL;
	}
	else {
		pf = fopen(savefilename, "wb+");
		fwrite(lsctabtoisp, 1, Width * Height * sizeof(int32_t), pf);
		fclose(pf);
		pf = NULL;
	}
	printf("Save Tab Done\n");
	return 1;
}

int isp_bayer_alsc_sinpi(int lut_in) {
	//{0	201	401	601	799	995	1189	1380	1567	1751	1931	2106	2276	2440	2598	2751	2896	3035	3166	3290	3406	3513	3612	3703	3784	3857	3920	3973	4017	4052	4076	4091	4096	4091	4076	4052	4017	3973	3920	3857	3784	3703	3612	3513	3406	3290	3166	3035	2896	2751	2598	2440	2276	2106	1931	1751	1567	1380	1189	995	799	601	401	201	0}
	int lut_sin[65] = {0,201,401,601,799,995,1189,1380,1567,1751,1931,2106,2276,2440,2598,2751,2896,3035,3166,3290,3406,3513,3612,3703,3784,3857,3920,3973,4017,4052,4076,4091,4096,4091,4076,4052,4017,3973,3920,3857,3784,3703,3612,3513,3406,3290,3166,3035,2896,2751,2598,2440,2276,2106,1931,1751,1567,1380,1189,995,799,601,401,201,0 };
	int lut_inabs = abs(lut_in);     // range: [-1,1], [-pi, pi]
	int left = lut_inabs >> 4;
	int right = left + 1;
	int weight = lut_inabs - (left << 4);
	left = min(left, 64);
	right = min(right, 64);
	int sinx = (lut_sin[left] * (16 - weight) + lut_sin[right] * weight) >> 4;
	if (lut_in < 0) {
		sinx = -sinx;
	}
	return sinx;
}

int isp_bayer_alsc_cospi(int lut_in) {
	//{0	201	401	601	799	995	1189	1380	1567	1751	1931	2106	2276	2440	2598	2751	2896	3035	3166	3290	3406	3513	3612	3703	3784	3857	3920	3973	4017	4052	4076	4091	4096	4091	4076	4052	4017	3973	3920	3857	3784	3703	3612	3513	3406	3290	3166	3035	2896	2751	2598	2440	2276	2106	1931	1751	1567	1380	1189	995	799	601	401	201	0}
	int lut_cos[65] = {4096,4091,4076,4052,4017,3973,3920,3857,3784,3703,3612,3513,3406,3290,3166,3035,2896,2751,2598,2440,2276,2106,1931,1751,1567,1380,1189,995,799,601,401,201,0,-201,-401,-601,-799,-995,-1189,-1380,-1567,-1751,-1931,-2106,-2276,-2440,-2598,-2751,-2896,-3035,-3166,-3290,-3406,-3513,-3612,-3703,-3784,-3857,-3920,-3973,-4017,-4052,-4076,-4091,-4096 };
	int lut_inabs = abs(lut_in);     // range: [-1,1], [-pi, pi]
	int left = lut_inabs >> 4;
	int right = left + 1;
	int weight = lut_inabs - (left << 4);
	left = min(left, 64);
	right = min(right, 64);
	int cosx = (lut_cos[left] * (16 - weight) + lut_cos[right] * weight) >> 4;
	return cosx;
}

int isp_bayer_alsc_arctan(int lut_in) {
	int lut_artan[161] = {0,32,64,96,127,159,190,221,251,281,310,339,367,395,422,449,475,500,525,549,572,595,617,638,659,679,699,718,736,754,771,788,804,820,835,850,864,878,892,905,918,930,942,953,965,976,986,996,1006,1016,1026,1035,1044,1052,1061,1069,1077,1085,1092,1100,1107,1114,1121,1127,1134,1140,1146,1152,1158,1164,1169,1175,1180,1185,1191,1196,1200,1205,1210,1214,1219,1223,1227,1232,1236,1240,1244,1248,1251,1255,1259,1262,1266,1269,1273,1276,1279,1282,1285,1288,1291,1294,1297,1300,1303,1306,1308,1311,1314,1316,1319,1321,1324,1326,1328,1331,1333,1335,1337,1339,1342,1344,1346,1348,1350,1352,1354,1356,1358,1360,1361,1363,1365,1367,1368,1370,1372,1374,1375,1377,1378,1380,1382,1383,1385,1386,1388,1389,1390,1392,1393,1395,1396,1397,1399,1400,1401,1403,1404,1405,1406};
	int lut_inabs = abs(lut_in);     
	int left = lut_inabs >> 3;
	int right = left + 1;
	int weight = lut_inabs - (left << 3);
	left = min(left, 160);
	right = min(right, 160);
	int artanx = (lut_artan[left] * (8 - weight) + lut_artan[right] * weight) >> 3;
	if (lut_in < 0)
		artanx = -artanx;
	return artanx;
}


int isp_bayer_alsc_calc_ind(struct isp_bayer_alsc_stat_fw* alsc_state, int imagew, int imageh, struct isp_bayer_alsc_tune_fw* alsc_tune, struct isp_bayer_alsc_inter_fw* alsc_inter) {

	int sum_ave = 0;
	int lenx = imagew / alsc_state->state_w;
	int leny = imageh / alsc_state->state_h;
	int crop_x = (imagew - lenx * alsc_state->state_w) >> 1;
	int crop_y = (imageh - leny * alsc_state->state_h) >> 1;
	//int crop_w, crop_h;
	int startx, starty;
	int indxi, indx;
	int diff;
	int tmp;
	bool indxdark = 0;
	for (int i = 0; i < alsc_state->state_h; i++) {
		indxi = i * alsc_state->state_w;
		for (int j = 0; j < alsc_state->state_w; j++) {
			indx = indxi + j;
			starty = crop_y + i * leny;
			startx = crop_x + j * lenx;
			sum_ave = 0;
			for (int ii = starty; ii < starty+leny; ii++) {
				for (int jj = startx; jj < startx+lenx; jj++) {
					tmp = imageh / 3 + ((imagew / 2 - imageh / 3)*alsc_tune->indx_ratio>>8);
					diff = (jj - (imagew >> 1)) * (jj - (imagew >> 1)) + (ii - (imageh >> 1)) * (ii - (imageh >> 1)) - tmp*tmp ;
					if (diff<5000) {
						sum_ave = sum_ave + 1;
					}	
				}
			}
			indxdark = (alsc_state->state_rbuffer[indx] > alsc_tune->th_darnum) && (alsc_state->state_grbuffer[indx] > alsc_tune->th_darnum) && (alsc_state->state_gbbuffer[indx] > alsc_tune->th_darnum) && (alsc_state->state_bbuffer[indx] > alsc_tune->th_darnum);
			alsc_inter->index[indx] = (sum_ave>alsc_tune->th_pixnum) && (alsc_state->state_wbuffer[indx] > alsc_tune->th_saunum) && indxdark;
		}
	}
	return 1;
};

int isp_bayer_alsc_stat_inv(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_storecrb_fw* alsc_tabcrb, struct isp_bayer_alsc_tune_fw* alsc_tune) {
	int indx;
	for (int i = 0; i < alsc_state->state_h; i++) {
		for (int j = 0; j < alsc_state->state_w; j++) {
			indx = i * alsc_state->state_w + j;
			if (alsc_tune->statr_inv) {
				alsc_state->state_rbuffer[indx] = (alsc_state->state_rbuffer[indx]) * alsc_tabcrb->tab_cr[indx]>>12;
			}
			if (alsc_tune->statb_inv) {
				alsc_state->state_bbuffer[indx] = (alsc_state->state_bbuffer[indx]) * alsc_tabcrb->tab_cb[indx]>>12;
			}	
		}
	}
	return 1;
}

int isp_bayer_alsc_calc_hue(struct isp_bayer_alsc_stat_fw* alsc_state, isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_inter_fw* alsc_inter) {
	int ave_r = 0;
	int ave_gr = 0;
	int ave_gb = 0;
	int ave_g = 0;
	int ave_b = 0;
	int rgain = alsc_calib->awbgain_r;   //3.12
	int bgain = alsc_calib->awbgain_b;   //3.12
	int hue_r;
	int hue_b;
	int pixnum=0;
	int indx = 0;
	for (int i = 0; i < alsc_state->state_h; i++) {
		for (int j = 0; j < alsc_state->state_w; j++) {
			indx = i * alsc_state->state_w + j;
			pixnum = max(alsc_state->state_wbuffer[indx], 1);
			ave_r = ((alsc_state->state_rbuffer[indx] << 1) + pixnum) / (pixnum << 1);
			ave_gr = ((alsc_state->state_grbuffer[indx] << 1) + pixnum) / (pixnum << 1);
			ave_gb = ((alsc_state->state_gbbuffer[indx] << 1) + pixnum) / (pixnum << 1);
			ave_b = ((alsc_state->state_bbuffer[indx] << 1) + pixnum) / (pixnum << 1);

			//ave_gr = alsc_state->state_grbuffer[indx] / max(alsc_state->state_wbuffer[indx], 1);
			//ave_gb = alsc_state->state_gbbuffer[indx] / max(alsc_state->state_wbuffer[indx], 1);
			//ave_b = alsc_state->state_bbuffer[indx] / max(alsc_state->state_wbuffer[indx], 1);
			ave_g = (ave_gr + ave_gb);
			if (ave_g > 0) {
				hue_r = ((ave_r * rgain<<2) + ave_g) / (ave_g<<1);  //13.12
				hue_b = ((ave_b * bgain<<2) + ave_g) / (ave_g<<1);  //13.12
				//if (i == 2) {
				//	printf("i=%d,j=%d,rv=%d,gr=%d,gb=%d,wb=%d,aver=%d,aveg=%d,huer=%d\n",i,j, alsc_state->state_rbuffer[indx], alsc_state->state_grbuffer[indx], alsc_state->state_gbbuffer[indx], alsc_state->state_wbuffer[indx],ave_r,ave_g,hue_r);
				//}
				//printf("i=%d,j=%d,rv=%d,gr=%d,gb=%d,wb=%d,aver=%d,aveg=%d,huer=%d\n",i,j, alsc_state->state_rbuffer[indx], alsc_state->state_grbuffer[indx], alsc_state->state_gbbuffer[indx], alsc_state->state_wbuffer[indx],ave_r,ave_g,hue_r);
			}			
			else {
				hue_r = 0;
				hue_b = 0;
			}
			alsc_inter->hue_r[indx] = clip(hue_r, 0, 1 << 20);    //8.12
			alsc_inter->hue_b[indx] = clip(hue_b, 0, 1 << 20);
		}
	}
	return 1;
};

int isp_bayer_alsc_calc_tab2(struct isp_bayer_alsc_stat_fw* alsc_state,
	struct isp_bayer_alsc_inter_fw* alsc_inter,
	struct isp_bayer_alsc_lsctabawb_fw* alsc_calib,
	struct isp_bayer_alsc_tune_fw* alsc_tune,
	int32_t gradr_th, int32_t* hue_rb, int32_t* tab_crb) {

	int statw = alsc_state->state_w;

	int indxi, indx;
	int indx01, indx10;
	int gradx_r, grady_r;
	int validx_flag, validy_flag;
	int64_t axint00=0, axint01=0, axint10=0, axint11=0;
	int64_t axinv00 = 0, axinv01 = 0, axinv10 = 0, axinv11 = 0;
	int64_t abxint0=0, abxint1=0;
	int64_t px0int = 0, px1int = 0;
	int64_t divxint = 0;

	int64_t ayint00 = 0, ayint01 = 0, ayint10 = 0, ayint11 = 0;
	int64_t ayinv00 = 0, ayinv01 = 0, ayinv10 = 0, ayinv11 = 0;
	int64_t abyint0 = 0, abyint1 = 0;
	int64_t py0int = 0, py1int = 0;
	int64_t divyint = 0;
	int ax, ay, axy, a_value;
	int phax, phay, phaxdi, phaydi;
	int cosphax, cosphay;
	for (int i = 0; i < alsc_state->state_h; i++) {
		indxi = i * alsc_state->state_w;
		for (int j = 0; j < alsc_state->state_w; j++) {
			indx = indxi + j;
			if (j == alsc_state->state_w - 1) {
				indx01 = indxi;
			}
			else {
				indx01 = indxi + j + 1;
			}
			if (i == alsc_state->state_h - 1) {
				indx10 = j;
			}
			else {
				indx10 = indxi + alsc_state->state_w + j;
			}
			//gradx_r = alsc_inter->hue_r[indx01] - alsc_inter->hue_r[indx];
			//grady_r = alsc_inter->hue_r[indx10] - alsc_inter->hue_r[indx];
			gradx_r = hue_rb[indx01] - hue_rb[indx];
			grady_r = hue_rb[indx10] - hue_rb[indx];
			// function
			int iuse = i + 1 + (alsc_state->state_w >> 1) - (alsc_state->state_h >> 1);
			int juse = j + 1;
			//int xxc = (juse << 10) / alsc_state->state_w;
			//int yyc = (iuse << 10) / alsc_state->state_w;
			int xxc = ((juse << 11) + statw) / (statw << 1);
			int yyc = ((iuse << 11) + statw) / (statw << 1);
			int sinx = isp_bayer_alsc_sinpi(xxc);                          //.12
			int cosx = isp_bayer_alsc_cospi(xxc);
			int siny = isp_bayer_alsc_sinpi(yyc);
			int cosy = isp_bayer_alsc_cospi(yyc);
			alsc_inter->tab_sinx[indx] = sinx;
			alsc_inter->tab_siny[indx] = siny;
			alsc_inter->tab_cosx[indx] = cosx;
			alsc_inter->tab_cosy[indx] = cosy;
			//int use_x1 = cosx * 804 / alsc_state->state_w >> 8;                //.12
			//int use_x2 = sinx * 804 / alsc_state->state_w >> 8;
			//int use_y1 = cosy * 804 / alsc_state->state_w >> 8;
			//int use_y2 = siny * 804 / alsc_state->state_w >> 8;                //round

			int use_x1 = ((cosx * 804 << 1) + statw) / (statw << 1) >> 8;                //.12
			int use_x2 = ((sinx * 804 << 1) + statw) / (statw << 1) >> 8;
			int use_y1 = ((cosy * 804 << 1) + statw) / (statw << 1) >> 8;
			int use_y2 = ((siny * 804 << 1) + statw) / (statw << 1) >> 8;                //round
			//lsm

			validx_flag = alsc_inter->index[indx]  & (abs(gradx_r) < gradr_th);    
			validy_flag = alsc_inter->index[indx]  & (abs(grady_r) < gradr_th);

			alsc_inter->debug_validx[indx] = validx_flag;
			alsc_inter->debug_validy[indx] = validy_flag;
			alsc_inter->debug_gradx[indx] = gradx_r;
			alsc_inter->debug_grady[indx] = grady_r;

			if (validx_flag == 1) {
				axint00 = axint00 + use_x1 * use_x1;     //.24+12
				axint01 = axint01 + use_x1 * use_x2;
				//axint10 = axint01;
				axint11 = axint11 + use_x2 * use_x2;
				abxint0 = abxint0 + use_x1 * gradx_r;
				abxint1 = abxint1 + use_x2 * gradx_r;
			}
			if (validy_flag == 1) {
				ayint00 = ayint00 + use_y1 * use_y1;
				ayint01 = ayint01 + use_y1 * use_y2;
				//axint10 = axint01;
				ayint11 = ayint11 + use_y2 * use_y2;
				abyint0 = abyint0 + use_y1 * grady_r;
				abyint1 = abyint1 + use_y2 * grady_r;
			}	
		}
	}

	
	int64_t axint00_ = axint00 >> 6;      //12.18
	int64_t axint01_ = axint01 >> 6;
	int64_t axint10_ = axint01;
	int64_t axint11_ = axint11 >> 6;
	int64_t abxint0_ = abxint0 >> 12;      //12.12
	int64_t abxint1_ = abxint1 >> 12;      //12.12
	divxint = axint00_ * axint11_ - axint01_ * axint01_;                 //s24.36
	int64_t divxintshift = divxint >> 18;
	if (divxintshift != 0) {
		//axinv00 = (axint11 << 2) / (divxint >> 18);                     //(12.24<<2) /(24.36>>18)-> .8
		//axinv01 = -(axint01 << 2) / (divxint >> 18);                     //(12.24<<2) /(24.36>>18)-> .8

		axinv00 =  ((axint11 << 3) + divxintshift) / (divxintshift<<1);                     //(12.24<<2) /(24.36>>18)-> .8
		axinv01 = -((axint01 << 3) + divxintshift) / (divxintshift<<1);                     //(12.24<<2) /(24.36>>18)-> .8

		axinv10 = axinv01;
		axinv11 = ((axint00 << 3) + divxintshift) / (divxintshift << 1);                     //FIXME mayebe need clip
		//axinv11 = (axint00 << 2) / (divxint >> 18);                     //FIXME mayebe need clip
		px0int = axinv00 * abxint0_ + axinv01 * abxint1_;                     //.20
		px1int = axinv10 * abxint0_ + axinv11 * abxint1_;
		px0int = px0int >> 10;                                               //.10
		px1int = px1int >> 10;
		px0int = clip(px0int, -(1 << 20), 1 << 20);                          //s10.10
		px1int = clip(px1int, -(1 << 20), 1 << 20);                          //s10.10
		int atanpx = 0;
		if (px0int != 0) {
			//atanpx = -(px1int << 8) / px0int;                            //.8
			atanpx = -((px1int << 9) + px0int) / (px0int<<1);                            //.8
		}
		//int atanpx = -(px1int << 8) / px0int;                                //.8
		phax = isp_bayer_alsc_arctan(atanpx);                            //.10
		phaxdi = phax * 326 >> 10;
		cosphax = isp_bayer_alsc_cospi(phaxdi);                //s1.12
		int px0int_clip = clip(px0int, -(1 << 12), 1 << 12);                 //s2.10
		if (cosphax != 0) {
			//ax = (px0int_clip << 14) / cosphax;                              //s2.12
			ax = ((px0int_clip << 15) + cosphax) / (cosphax<<1);                              //s2.12
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}

	int64_t ayint00_ = ayint00 >> 6;      //12.18
	int64_t ayint01_ = ayint01 >> 6;
	int64_t ayint10_ = ayint01;
	int64_t ayint11_ = ayint11 >> 6;
	int64_t abyint0_ = abyint0 >> 12;      //12.12
	int64_t abyint1_ = abyint1 >> 12;      //12.12
	divyint = ayint00_ * ayint11_ - ayint01_ * ayint01_;                 //s24.36
	int64_t divyintshift = divyint >> 18;
	if (divyint != 0) {
		//ayinv00 = (ayint11 << 2) / (divyint >> 18);                     //(12.24<<2) /(24.36>>18)-> .8
		//ayinv01 = -(ayint01 << 2) / (divyint >> 18);                     //(12.24<<2) /(24.36>>18)-> .8
		//ayinv10 = ayinv01;
		//ayinv11 = (ayint00 << 2) / (divyint >> 18);                     //FIXME mayebe need clip

		ayinv00 =  ((ayint11 << 3) + divyintshift) / (divyintshift << 1);                     //(12.24<<2) /(24.36>>18)-> .8
		ayinv01 = -((ayint01 << 3) + divyintshift) / (divyintshift << 1);                     //(12.24<<2) /(24.36>>18)-> .8
		ayinv10 =    ayinv01;
		ayinv11 =  ((ayint00 << 3) + divyintshift) / (divyintshift << 1);                     //FIXME mayebe need clip

		py0int = ayinv00 * abyint0_ + ayinv01 * abyint1_;                     //.20
		py1int = ayinv10 * abyint0_ + ayinv11 * abyint1_;
		py0int = py0int >> 10;                                               //.10
		py1int = py1int >> 10;
		py0int = clip(py0int, -(1 << 20), 1 << 20);                          //s10.10
		py1int = clip(py1int, -(1 << 20), 1 << 20);                          //s10.10
		int atanpy = 0;
		if (py0int != 0) {
			atanpy = -((py1int << 9)+ py0int) / (py0int<<1);                            //.8
		}
		phay = isp_bayer_alsc_arctan(atanpy);                            //.10
		phaydi = phay * 326 >> 10;
		cosphay = isp_bayer_alsc_cospi(phaydi);                //s1.12
		int py0int_clip = clip(py0int, -(1 << 12), 1 << 12);                 //s2.10
		if (cosphay != 0) {
			ay = ((py0int_clip << 15)+ cosphay) / (cosphay<<1);                              //s2.12
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}

	int sinxpha, sinypha;
	int gxf, gyf;
	if (ax > 0 && ax < (1 << 13) && ay>0 && ay < (1 << 13)) {               // really  NEED this?
		axy = (ax + ay) >> 1;                                               //s2.12
		a_value = sqrt(axy);                                                //s1.6
		int sinphax = isp_bayer_alsc_sinpi(phaxdi);                         //s1.12
		int sinphay = isp_bayer_alsc_sinpi(phaydi);                         //s1.12
		for (int i = 0; i < alsc_state->state_h; i++) {
			indxi = i * alsc_state->state_w;
			for (int j = 0; j < alsc_state->state_w; j++) {
				indx = indxi + j;
				sinxpha = alsc_inter->tab_sinx[indx] * cosphax + alsc_inter->tab_cosx[indx] * sinphax;
				sinxpha = sinxpha >> 12;                           //s2.12
				sinypha = alsc_inter->tab_siny[indx] * cosphay + alsc_inter->tab_cosy[indx] * sinphay;
				sinypha = sinypha >> 12;                           //s2.12
				gxf = a_value * sinxpha >> 6;                      //.12
				gyf = a_value * sinypha >> 6;                      //.12
				tab_crb[indx] = (gxf * gyf >> 12) + (1 << 12) - axy;
			}
		}
	}
	else {
		return 0;
	}

	return 1;
}



int isp_bayer_alsc_calc_tab(struct isp_bayer_alsc_stat_fw* alsc_state, 
							struct isp_bayer_alsc_inter_fw* alsc_inter, 
							struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, 
							struct isp_bayer_alsc_tune_fw* alsc_tune,
							int32_t gradr_th, int32_t* hue_rb, int32_t* tab_crb) {
	int indxi = 0;
	int indx = 0;
	int indx10 = 0;
	int indx01 = 0;
	int gradx_r;
	int grady_r;

	int validx_flag = 0;
	int validy_flag = 0;
	int ax_sum = 0;
	int ax_w = 0;
	int ay_sum = 0;
	int ay_w = 0;
	int ax_value = 0;            //s6.12
	int ay_value = 0;            //s6.12
	int axy_value = 0;
	int a_value = 0;             //1.6
	for (int i = 0; i < alsc_state->state_h; i++) {
		indxi = i * alsc_state->state_w;
		for (int j = 0; j < alsc_state->state_w; j++) {
			indx = indxi + j;
			if (j == alsc_state->state_w - 1) {
				indx01 = indxi;
			}
			else {
				indx01 = indxi + j + 1;
			}
			if (i == alsc_state->state_h - 1) {
				indx10 = j;
			}
			else {
				indx10 = indxi + alsc_state->state_w + j;
			}			
			//gradx_r = alsc_inter->hue_r[indx01] - alsc_inter->hue_r[indx];
			//grady_r = alsc_inter->hue_r[indx10] - alsc_inter->hue_r[indx];
			gradx_r = hue_rb[indx01] - hue_rb[indx];
			grady_r = hue_rb[indx10] - hue_rb[indx];


			// function
			int iuse = i + 1 + (alsc_state->state_w>>1) - (alsc_state->state_h>>1);
			int juse = j + 1;
			int shiftx = (alsc_tune->mcenter_x << 10) / alsc_calib->image_w;       //.10
			int shifty = (alsc_tune->mcenter_y << 10) * alsc_state->state_h / alsc_state->state_w / alsc_calib->image_h;       //.10
			int xxc = (juse << 10) / alsc_state->state_w - shiftx;
			int yyc = (iuse << 10) / alsc_state->state_w - shifty;
			int sinx = isp_bayer_alsc_sinpi(xxc);                          //.12
			int cosx = isp_bayer_alsc_cospi(xxc);
			int siny = isp_bayer_alsc_sinpi(yyc);
			int cosy = isp_bayer_alsc_cospi(yyc);
			alsc_inter->tab_sinx[indx] = sinx;
			alsc_inter->tab_siny[indx] = siny;
			int sinycosx = siny * cosx >> 12;                             //.12
			int sinxcosy = sinx * cosy >> 12;
			int use_x = sinycosx * 804 / alsc_state->state_w;
			int use_y = sinxcosy * 804 / alsc_state->state_w;
			use_x = use_x >> 8;                                        //.12
			use_y = use_y >> 8;                                        //.12

			//printf("i=%d, j=%d, iuse=%d, juse=%d,siny=%d, cosx=%d, sinx=%d, cosy=%d, gradx_r=%d, use_x=%d, use_y=%d, gradx_r=%d\n", i, j, iuse,juse,siny, cosx, sinx, cosy,gradx_r, use_x, use_y, gradx_r);
			//int abs(gradx_r) < gradx_th;
			//int tmp_flag = abs(use_x) > alsc_tune->div_th;
			validx_flag = alsc_inter->index[indx] & (abs(use_x) > alsc_tune->div_th) & (abs(gradx_r) < gradr_th);    //FIXME
			validy_flag = alsc_inter->index[indx] & (abs(use_y) > alsc_tune->div_th) & (abs(grady_r) < gradr_th);
			//printf("%d ", validx_flag);
			if (validx_flag) {
				ax_value = (gradx_r << 12) / use_x;                               //.12
				//ax_value = clip(ax_value, -((1 << 18)-1), (1 << 18)-1);           //s6.12
				ax_sum = ax_sum + ax_value;
				ax_w = ax_w + 1;
			}
			if (validy_flag) {
				ay_value = (grady_r << 12) / use_y;                               //.12
				//ay_value = clip(ay_value, -((1 << 18) - 1), (1 << 18) - 1);           //s6.12
				ay_sum = ay_sum + ay_value;
				ay_w = ay_w + 1;
			}		

		}
		//printf("\n");
	}

	if (ax_w > 0) {
		ax_value = ax_sum / ax_w;
	}
	else {
		return 0;
	}
	if (ay_w > 0) {
		ay_value = ay_sum / ay_w;
	}
	else {
		return 0;
	}
	// ax and ay 1.12 is valid
	int gxf = 0;
	int gyf = 0;
	int gf = 0;
	if (ax_value > 0 && ax_value < (1 << 13) && ay_value>0 && ay_value < (1 << 13)) {
		axy_value = (ax_value + ay_value) >> 1;
		a_value = sqrt(axy_value);               // 1.6          //maybe need fix sqrt
		for (int i = 0; i < alsc_state->state_h; i++) {
			indxi = i * alsc_state->state_w;
			for (int j = 0; j < alsc_state->state_w; j++) {
				indx = indxi + j;
				gxf = a_value * alsc_inter->tab_sinx[indx] >> 6;       //1.12
				gyf = a_value * alsc_inter->tab_siny[indx] >> 6;       //1.12
				gf = (gxf * gyf >> 12) + (1 << 12) - axy_value;        //.12
				// maybe need FIXME  THIS max value 
				//alsc_inter->tab_cr[indx] = gf;
				tab_crb[indx] = gf;
			}
		}
		// cal end
	}
	else {
		return 0;    // use lsc 
	}

	return 1;
};

int isp_bayer_alsc_calc_wbpad(struct isp_bayer_alsc_stat_fw* alsc_state, 
	                          struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, 
	                          struct isp_bayer_alsc_tune_fw* alsc_tune,
							  int* tab_hue, int* tab_crb) {
	int tt_val=0;
	int64_t tt_sum = 0;
	int tt_w = 0;
	int indxi, indx;
	if (alsc_tune->white_mode) {
		for (int i = 0; i < alsc_state->state_h; i++) {
			indxi = i * alsc_state->state_w;
			for (int j = 0; j < alsc_state->state_w; j++) {
				indx = indxi + j;
				if (alsc_calib->white_indx[indx] == 1) {
					tt_val = tab_hue[indx] - tab_crb[indx];
					tt_sum = tt_sum + tt_val;
					tt_w = tt_w + 1;
				}
			}
		}
		// update this
		if (tt_w > 0) {
			tt_val = tt_sum / tt_w;
			if (tt_val > 0) {
				for (int i = 0; i < alsc_state->state_h; i++) {
					indxi = i * alsc_state->state_w;
					for (int j = 0; j < alsc_state->state_w; j++) {
						indx = indxi + j;
						tab_crb[indx] = ((tab_crb[indx] - (1 << 12)) << 12) / tt_val + (1 << 12);
					}
				}
			}	
		}
	}

	// padmode
	int ipad, jpad, indxipad;
	if (alsc_tune->pad_mode) {
		for (int i = 0; i < alsc_state->state_h; i++) {
			indxi = i * alsc_state->state_w;
			ipad = clip(i, alsc_tune->pad_h, alsc_state->state_h - alsc_tune->pad_h - 1);
			indxipad = ipad * alsc_state->state_w;
			for (int j = 0; j < alsc_state->state_w; j++) {
				indx = indxi + j;
				jpad = clip(j, alsc_tune->pad_w, alsc_state->state_w - alsc_tune->pad_w - 1);
				tab_crb[indx] = tab_crb[indxipad + jpad];
			}
		}
	}
	return 1;
};

int isp_bayer_alsc_updatestore(int state_w, int state_h, int32_t* tabcrb, int32_t* storecrb) {

	memcpy(storecrb, tabcrb, state_w * state_h*4);

	//int indxi, indx;
	//for (int i = 0; i < state_h; i++) {
	//	indxi = i * state_w;
	//	for (int j = 0; j < state_w; j++) {
	//		indx = indxi + j;
	//		storecrb[indx] = tabcrb[indx];
	//	}
	//}
	return 1;
}

int isp_bayer_alsc_update(struct isp_bayer_alsc_stat_fw* alsc_state, 
	                      struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, 
						  struct isp_bayer_alsc_inter_fw* alsc_inter,
						  int* tabg, int* tabcrb, int* tabcrb_out) {
	// scaler
	//int input_width, intput_height, output_width, output_height;
	
	int phasew, phaseh;                                                  //8.8
	int step_w = (alsc_state->state_w << 8) / alsc_inter->block_w;       //8.8    
	int step_h = (alsc_state->state_h << 8) / alsc_inter->block_h;
	int init_phasew = (step_w - (1 << 8)) >> 1;
	int init_phaseh = (step_h - (1 << 8)) >> 1; 

	int indxi, indxj, indxir, indxjr;
	int indxi_, indxir_;
	int disti, distj;
	int weight_ave = 0;
	int w11, w12, w21, w22;
	int indxouti = 0;
	for (int i = 0; i < alsc_inter->block_h; i++) {
		if (i == 0) {
			phaseh = init_phaseh;
		}
		else {
			phaseh = phaseh + step_h;
		}
		indxi = phaseh >> 8;
		disti = phaseh & 0xFF;
		indxir = clip(indxi + 1, 0, alsc_state->state_h-1);
		indxi = clip(indxi, 0, alsc_state->state_h-1);
		indxi_ = indxi * alsc_state->state_w;
		indxir_ = indxir * alsc_state->state_w;
		indxouti = i * alsc_inter->block_w;
		for (int j = 0; j < alsc_inter->block_w; j++) {
			// scaler
			if (j == 0) {
				phasew = init_phasew;
			}
			else {
				phasew = phasew + step_w;
			}
			indxj = phasew >> 8;
			distj = phasew & 0xFF;
			indxjr = clip(indxj + 1, 0, alsc_state->state_w-1);
			indxj = clip(indxj, 0, alsc_state->state_w-1);
			w11 = (((1 << 8) - disti) * ((1 << 8) - distj)) >> 4;   //.12
			w12 = (((1 << 8) - disti) * (distj)) >> 4;              //.12
			w21 = (disti * ((1 << 8) - distj)) >> 4;                //.12
			w22 = (disti * distj) >> 4;                             //.12
			//weight_ave = (alsc_inter->tab_cr[indxi_ + indxj] * w11 + alsc_inter->tab_cr[indxi_ + indxjr] * w12 +
			//	alsc_inter->tab_cr[indxir_ + indxj] * w21 + alsc_inter->tab_cr[indxir_ + indxjr] * w22) >> 12;
			weight_ave = (tabcrb[indxi_ + indxj] * w11 + tabcrb[indxi_ + indxjr] * w12 +
				tabcrb[indxir_ + indxj] * w21 + tabcrb[indxir_ + indxjr] * w22) >> 12;
			// update
			tabcrb_out[indxouti +j] = (tabg[indxouti + j]<<12) / weight_ave;   // need update this
		}
	}
	return 1;
};

int isp_bayer_alsc_init(struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, 
						struct isp_bayer_alsc_tune_fw* alsc_tune, 
						struct isp_bayer_alsc_inter_fw* alsc_inter, 
						struct isp_bayer_alsc_out_fw* alsc_out,
						struct isp_bayer_alsc_caltab_fw* alsc_calibtab
) {
	
	alsc_inter->block_w = alsc_calibtab->block_w;
	alsc_inter->block_h = alsc_calibtab->block_h;

	// gradient th
	int gradr_th = 0;
	//if (alsc_tune->manul_mode==0) {
	//	gradr_th = alsc_calib->grad_th_r;
	//}
	//else if(alsc_tune->manul_mode == 1){
	//	gradr_th = alsc_tune->mgrad_th_r;
	//}
	//else {
	//	gradr_th = (alsc_calib->grad_th_r*alsc_tune->mgrad_th_r)>>8;
	//}

	int gradb_th = 0;
	//if (alsc_tune->manul_mode == 0) {
	//	gradb_th = alsc_calib->grad_th_b;
	//}
	//else if (alsc_tune->manul_mode == 1) {
	//	gradb_th = alsc_tune->mgrad_th_b;
	//}
	//else {
	//	gradb_th = (alsc_calib->grad_th_b * alsc_tune->mgrad_th_b) >> 8;
	//}
	gradr_th = alsc_tune->mgrad_th_r;
	gradb_th = alsc_tune->mgrad_th_b;
	alsc_inter->grad_thr = gradr_th;
	alsc_inter->grad_thb = gradb_th;

	// out ini
	int indxi, indx;
	for (int i = 0; i < alsc_inter->block_h; i++) {
		indxi = i * alsc_inter->block_w;
		for (int j = 0; j < alsc_inter->block_w; j++) {
			indx = indxi + j;
			alsc_inter->tab_r[indx] = 1 << 12;
			alsc_inter->tab_g[indx] = 1 << 12;
			alsc_inter->tab_b[indx] = 1 << 12;
			alsc_out->lsctab_r[indx] = 1 << 12;
			alsc_out->lsctab_gr[indx] = 1 << 12;
			alsc_out->lsctab_gb[indx] = 1 << 12;
			alsc_out->lsctab_b[indx] = 1 << 12;
		}
	}
	return 1;
};

int isp_bayer_alsc_dlsc(struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_caltab_fw* alsc_calibtab, struct isp_bayer_alsc_inter_fw* alsc_inter, struct isp_bayer_alsc_out_fw* alsc_out) {
	int ti = 0;
	for (ti = 0; ti < 7; ti++) {
		if (alsc_calibtab->colort_list[ti]> alsc_calib->corlort) {
			break;
		}
	}
	int indxi, indx;
	int wleft, wright, wsum;
	int leftvr, rightvr;
	int leftvg, rightvg;
	int leftvb, rightvb;
	//float tmpv = 0;
	if (ti == 0) {
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				alsc_inter->tab_r[indx] = (int)(alsc_calibtab->tab1_r[indx] * (1<<12));
				alsc_inter->tab_g[indx] = (int)((alsc_calibtab->tab1_gr[indx] + alsc_calibtab->tab1_gb[indx]) * (1 << 11));
				alsc_inter->tab_b[indx] = (int)(alsc_calibtab->tab1_b[indx] * (1 << 12));
			}
		}
	}
	else if (alsc_calibtab->colort_list[6] < alsc_calib->corlort) {
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				alsc_inter->tab_r[indx] = (int)(alsc_calibtab->tab7_r[indx] * (1 << 12));
				alsc_inter->tab_g[indx] = (int)((alsc_calibtab->tab7_gr[indx] + alsc_calibtab->tab7_gb[indx]) * (1 << 11));
				alsc_inter->tab_b[indx] = (int)(alsc_calibtab->tab7_b[indx] * (1 << 12));
			}
		}
	}
	else if (ti == 1) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti-1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti-1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab1_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab2_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab1_gr[indx] + alsc_calibtab->tab1_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab2_gr[indx] + alsc_calibtab->tab2_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab1_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab2_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	else if (ti == 2) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti - 1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti - 1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab2_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab3_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab2_gr[indx] + alsc_calibtab->tab2_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab3_gr[indx] + alsc_calibtab->tab3_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab2_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab3_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	else if (ti == 3) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti - 1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti - 1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab3_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab4_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab3_gr[indx] + alsc_calibtab->tab3_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab4_gr[indx] + alsc_calibtab->tab4_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab3_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab4_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	else if (ti == 4) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti - 1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti - 1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab4_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab5_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab4_gr[indx] + alsc_calibtab->tab4_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab5_gr[indx] + alsc_calibtab->tab5_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab4_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab5_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	else if (ti == 5) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti - 1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti - 1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab5_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab6_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab5_gr[indx] + alsc_calibtab->tab5_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab6_gr[indx] + alsc_calibtab->tab6_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab5_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab6_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	else if (ti == 6) {
		wsum = alsc_calibtab->colort_list[ti] - alsc_calibtab->colort_list[ti - 1];
		wright = alsc_calib->corlort - alsc_calibtab->colort_list[ti - 1];
		wleft = alsc_calibtab->colort_list[ti] - alsc_calib->corlort;
		for (int i = 0; i < alsc_calibtab->block_h; i++) {
			indxi = i * alsc_calibtab->block_w;
			for (int j = 0; j < alsc_calibtab->block_w; j++) {
				indx = indxi + j;
				leftvr = (int)(alsc_calibtab->tab6_r[indx] * (1 << 12));
				rightvr = (int)(alsc_calibtab->tab7_r[indx] * (1 << 12));
				leftvg = (int)((alsc_calibtab->tab6_gr[indx] + alsc_calibtab->tab6_gb[indx]) * (1 << 11));
				rightvg = (int)((alsc_calibtab->tab7_gr[indx] + alsc_calibtab->tab7_gb[indx]) * (1 << 11));
				leftvb = (int)(alsc_calibtab->tab6_b[indx] * (1 << 12));
				rightvb = (int)(alsc_calibtab->tab7_b[indx] * (1 << 12));
				alsc_inter->tab_r[indx] = (leftvr * wleft + rightvr * wright) / wsum;
				alsc_inter->tab_g[indx] = (leftvg * wleft + rightvg * wright) / wsum;
				alsc_inter->tab_b[indx] = (leftvb * wleft + rightvb * wright) / wsum;
			}
		}
	}
	// out
	alsc_out->block_h = alsc_inter->block_h;
	alsc_out->block_w = alsc_inter->block_w;
	for (int i = 0; i < alsc_inter->block_h; i++) {
		indxi = i * alsc_calibtab->block_w;
		for (int j = 0; j < alsc_inter->block_w; j++) {
			indx = indxi + j;
			alsc_out->lsctab_r[indx] = alsc_inter->tab_r[indx];
			alsc_out->lsctab_gr[indx] = alsc_inter->tab_g[indx];
			alsc_out->lsctab_gb[indx] = alsc_inter->tab_g[indx];
			alsc_out->lsctab_b[indx] = alsc_inter->tab_b[indx];
		}
	}
	return 1;
};

int isp_bayer_alsc_process(struct isp_bayer_alsc_stat_fw* alsc_state, struct isp_bayer_alsc_lsctabawb_fw* alsc_calib, struct isp_bayer_alsc_tune_fw* alsc_tune, struct isp_bayer_alsc_caltab_fw* alsc_calibtab, struct isp_bayer_alsc_storecrb_fw* alsc_storecrb, float* lsctabtoisp) {
	//int arry_size = 7 * alsc_state->state_h * alsc_state->state_w + 5 * alsc_calib->block_h * alsc_calib->block_w;
	struct isp_bayer_alsc_inter_fw* alsc_inter = (struct isp_bayer_alsc_inter_fw *)malloc(sizeof(struct isp_bayer_alsc_inter_fw));
	struct isp_bayer_alsc_out_fw* alsc_out = (struct isp_bayer_alsc_out_fw*)malloc(sizeof(struct isp_bayer_alsc_out_fw));

	if (alsc_inter == NULL) {
		printf("malloc mem failed\n");
		return -1;
	}

	if (alsc_out == NULL) {
		printf("malloc mem failed\n");
		return -1;
	}

	//memset(alsc_inter, 0, sizeof(struct isp_bayer_alsc_inter_fw));

	isp_bayer_alsc_init(alsc_calib, alsc_tune, alsc_inter, alsc_out, alsc_calibtab);
	
	if (alsc_tune->enable>0) {
		// dlsc
		if (alsc_tune->enable == 1) {
			isp_bayer_alsc_dlsc(alsc_calib, alsc_calibtab, alsc_inter, alsc_out);
		}
		else {
			// no luma lsc
		}
		//alsc
		if (alsc_tune->alsc_enable) {
			isp_bayer_alsc_calc_ind(alsc_state, alsc_calib->image_w, alsc_calib->image_h, alsc_tune, alsc_inter);
			// state inv
			isp_bayer_alsc_stat_inv(alsc_state, alsc_storecrb, alsc_tune);
			// calcu inv
			isp_bayer_alsc_calc_hue(alsc_state, alsc_calib, alsc_inter);

			SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_index.bin", &alsc_inter->index[0], 48, 64, 0);
			SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_hue_r.bin", &alsc_inter->hue_r[0], 48, 64, 0);
			SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_hue_b.bin", &alsc_inter->hue_b[0], 48, 64, 0);
			if (alsc_tune->r_enable) {
				//ralsc  
				int rv=0;
				if (alsc_tune->alsc_mode == 0) {
					rv = isp_bayer_alsc_calc_tab(alsc_state, alsc_inter, alsc_calib, alsc_tune, alsc_inter->grad_thr, alsc_inter->hue_r, alsc_inter->tab_cr);
				}
				else if (alsc_tune->alsc_mode == 1) {
					rv = isp_bayer_alsc_calc_tab2(alsc_state, alsc_inter, alsc_calib, alsc_tune, alsc_inter->grad_thr, alsc_inter->hue_r, alsc_inter->tab_cr);
				}
				
				SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_tabcr.bin", &alsc_inter->tab_cr[0], 48, 64, 0);
				SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_validx.bin", &alsc_inter->debug_validx[0], 48, 64, 0);
				SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_validy.bin", &alsc_inter->debug_validy[0], 48, 64, 0);
				SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_gradx.bin", &alsc_inter->debug_gradx[0], 48, 64, 0);
				SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_grady.bin", &alsc_inter->debug_grady[0], 48, 64, 0);

				if (rv == 1) {
					isp_bayer_alsc_calc_wbpad(alsc_state, alsc_calib, alsc_tune, alsc_inter->hue_r, alsc_inter->tab_cr);
					SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_padcr.bin", &alsc_inter->tab_cr[0], 48, 64, 0);
					isp_bayer_alsc_updatestore(alsc_state->state_w, alsc_state->state_h, &alsc_inter->tab_cr[0], &alsc_storecrb->tab_cr[0]);
					isp_bayer_alsc_update(alsc_state, alsc_calib, alsc_inter, alsc_inter->tab_g, alsc_inter->tab_cr, alsc_out->lsctab_r);
					SaveAlscTab("D:\\Project\\JX10X\\alsc\\alsc\\alsc\\case\\debug\\debug_updatecr.bin", &alsc_out->lsctab_r[0], 48, 64, 0);
				}	
			}
			if (alsc_tune->b_enable) {
				//balsc
				int bv=0;
				if (alsc_tune->alsc_mode == 0) {
					bv = isp_bayer_alsc_calc_tab(alsc_state, alsc_inter, alsc_calib, alsc_tune, alsc_inter->grad_thb, alsc_inter->hue_b, alsc_inter->tab_cb);
				}
				else if(alsc_tune->alsc_mode==1){
					bv = isp_bayer_alsc_calc_tab2(alsc_state, alsc_inter, alsc_calib, alsc_tune, alsc_inter->grad_thb, alsc_inter->hue_b, alsc_inter->tab_cb);
				}

				if (bv == 1) {
					isp_bayer_alsc_calc_wbpad(alsc_state, alsc_calib, alsc_tune, alsc_inter->hue_b, alsc_inter->tab_cb);
					isp_bayer_alsc_updatestore(alsc_state->state_w, alsc_state->state_h, &alsc_inter->tab_cb[0], &alsc_storecrb->tab_cb[0]);
					isp_bayer_alsc_update(alsc_state, alsc_calib, alsc_inter, alsc_inter->tab_g, alsc_inter->tab_cb, alsc_out->lsctab_b);
				}
			}
		}
		else {
			// use dlsc
		}
	}
	else {
		// not lsc
	}

	int offsetgr = alsc_inter->block_h * alsc_inter->block_w;
	int offsetgb = offsetgr << 1;
	int offsetb = offsetgr + offsetgb;
	int indxi, indx;
	for (int i = 0; i < alsc_inter->block_h; i++) {
		indxi = i * alsc_inter->block_w;
		for (int j = 0; j < alsc_inter->block_w; j++) {
			indx = indxi + j;
			lsctabtoisp[indx] = alsc_out->lsctab_r[indx] * 1.0 / (1 << 12);
			lsctabtoisp[indx+offsetgr] = alsc_out->lsctab_gr[indx] * 1.0 / (1 << 12);
			lsctabtoisp[indx+offsetgb] = alsc_out->lsctab_gb[indx] * 1.0 / (1 << 12);
			lsctabtoisp[indx+offsetb] = alsc_out->lsctab_b[indx] * 1.0 / (1 << 12);
		}
	}

	free(alsc_out);
	free(alsc_inter);
	return 1;
};
