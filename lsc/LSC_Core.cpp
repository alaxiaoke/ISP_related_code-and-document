#include "LSC_Core.h"

int32_t tableUpScale(uint16_t* shading_table, uint16_t* upscale_table) {


    int table_h = 13;
    int table_w = 17;
    int out_h = RAW_H >> 1;
    int out_w = RAW_W >> 1;

    int step_h = (table_h << 14) / out_h;
    int step_w = (table_w << 14) / out_w;
    int init_phaseh = 0;     //.14
    int init_phasew = 0;     //.14
    int offset_indh = 0;
    int offset_indw = 0;


    int phasew, phaseh;
    int indxi, indxj, indxir, indxjr;
    int indxi_, indxir_;
    int disti, distj;
    int p00, p01, p10, p11;     //5bit

    int kerw00, kerw01, kerw10, kerw11;     //5bit
    int kerw1, kerw2, kerw;

    int indxouti = 0;
    for (int i = 0; i < out_h; i++) {
        //printf("i=%d\n", i);
        if (i == 0) {
            phaseh = init_phaseh;
        }
        else {
            phaseh = phaseh + step_h;
        }
        indxi = phaseh >> 14;
        disti = phaseh & 0x3FFF;
        indxi = indxi + offset_indh;
        indxir = CLIP(indxi + 1, 0, table_h - 1);
        indxi = CLIP(indxi, 0, table_h - 1);
        indxi_ = indxi * table_w;                                  // need offset
        indxir_ = indxir * table_w;
        indxouti = i * out_w;

        for (int j = 0; j < out_w; j++) {
            // scaler
            if (j == 0) {
                phasew = init_phasew;
            }
            else {
                phasew = phasew + step_w;
            }
            indxj = phasew >> 14;                              // need offset
            distj = phasew & 0x3FFF;
            indxj = indxj + offset_indw;
            indxjr = CLIP(indxj + 1, 0, table_w - 1);
            indxj = CLIP(indxj, 0, table_w - 1);

            p00 = shading_table[indxi_ + indxj];         //16bit                       
            p01 = shading_table[indxi_ + indxjr];        //16bit                         
            p10 = shading_table[indxir_ + indxj];        //16bit
            p11 = shading_table[indxir_ + indxjr];

            kerw1 = (p00 * ((1 << 14) - distj) + p01 * distj) >> 14;
            kerw2 = (p10 * ((1 << 14) - distj) + p11 * distj) >> 14;
            kerw = (kerw1 * ((1 << 14) - disti) + kerw2 * disti) >> 14;

            upscale_table[indxouti + j] = kerw;

        }
    }
    return 1;
}


void apply_lsc(int bayer_pat, LSC_UPSCALE_TABLE* upscale_table, uint16_t* data) {
    for (int i = 0; i < RAW_H/2; i++) {
        for (int j = 0; j < RAW_W/2; j++) {
            if (bayer_pat == GXR_Raw_RGGB) {
                data[2 * i * RAW_W + 2 * j] = data[2 * i * RAW_W + 2 * j] * upscale_table->upscale_table_r[i * RAW_W /2 + j] >> 12;
                data[2 * i * RAW_W + 2 * j + 1] = data[2 * i * RAW_W + 2 * j + 1] * upscale_table->upscale_table_gr[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j] = data[(2 * i + 1) * RAW_W + 2 * j] * upscale_table->upscale_table_gb[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j + 1] = data[(2 * i + 1) * RAW_W + 2 * j + 1] * upscale_table->upscale_table_b[i * RAW_W / 2 + j] >> 12;
            }
            if (bayer_pat == GXR_Raw_BGGR) {
                data[2 * i * RAW_W + 2 * j] = data[2 * i * RAW_W + 2 * j] * upscale_table->upscale_table_b[i * RAW_W / 2 + j] >> 12;
                data[2 * i * RAW_W + 2 * j + 1] = data[2 * i * RAW_W + 2 * j + 1] * upscale_table->upscale_table_gb[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j] = data[(2 * i + 1) * RAW_W + 2 * j] * upscale_table->upscale_table_gr[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j + 1] = data[(2 * i + 1) * RAW_W + 2 * j + 1] * upscale_table->upscale_table_r[i * RAW_W / 2 + j] >> 12;
            }
            if (bayer_pat == GXR_Raw_GRBG) {
                data[2 * i * RAW_W + 2 * j] = data[2 * i * RAW_W + 2 * j] * upscale_table->upscale_table_gr[i * RAW_W / 2 + j] >> 12;
                data[2 * i * RAW_W + 2 * j + 1] = data[2 * i * RAW_W + 2 * j + 1] * upscale_table->upscale_table_r[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j] = data[(2 * i + 1) * RAW_W + 2 * j] * upscale_table->upscale_table_b[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j + 1] = data[(2 * i + 1) * RAW_W + 2 * j + 1] * upscale_table->upscale_table_gb[i * RAW_W / 2 + j] >> 12;
            }
            if (bayer_pat == GXR_Raw_GBRG) {
                data[2 * i * RAW_W + 2 * j] = data[2 * i * RAW_W + 2 * j] * upscale_table->upscale_table_gb[i * RAW_W / 2 + j] >> 12;
                data[2 * i * RAW_W + 2 * j + 1] = data[2 * i * RAW_W + 2 * j + 1] * upscale_table->upscale_table_b[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j] = data[(2 * i + 1) * RAW_W + 2 * j] * upscale_table->upscale_table_r[i * RAW_W / 2 + j] >> 12;
                data[(2 * i + 1) * RAW_W + 2 * j + 1] = data[(2 * i + 1) * RAW_W + 2 * j + 1] * upscale_table->upscale_table_gr[i * RAW_W / 2 + j] >> 12;
            }
         }
    }



}