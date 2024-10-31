#include  <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "allib_LSC.h"
#include "alLSCLib_Error_Code.h"

using namespace std;

typedef struct al4ahandle {

    struct alLSClib_lib_version_t       lsc_ver;
    struct alLSClib_runtime_obj_t       lsc_func_obj;
    struct alLSCLib_set_parameter_t     lsc_set_para;
    struct alLSCLib_get_parameter_t     lsc_get_para;
    struct alLSClib_output_data_t       lsc_default_output_data;
    struct alLSClib_output_data_t       lsc_output;
    void* lsc_obj;

}al4ahd;


al4ahd          al4a;
unsigned int    g_udsofidx = 0;

unsigned int load_lib(al4ahd* pal4a);
unsigned int lsc_initial(al4ahd* pal4a);
unsigned int lsc_run(al4ahd* pal4a);

int main()
{
	int err = 0;
    alLSCLib_getlib_version(&(al4a.lsc_ver));

    err = load_lib(&al4a);

    err = lsc_initial(&al4a);
   
    err = lsc_run(&al4a);

    err = al4a.lsc_func_obj.deinit(al4a.lsc_obj);
    //*((volatile unsigned int *)0x20000034) = err;

    return 0;
}

/**********AE***********/


unsigned int load_lib(al4ahd* pal4a)
{
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;


    lsc_err = alLSCLib_loadfunc(&(pal4a->lsc_func_obj), 0);

    return 0;
}

unsigned int lsc_initial(al4ahd* pal4a)
{
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;

    void** lsc_obj = &(pal4a->lsc_obj);
    struct alLSClib_runtime_obj_t* lsc_func_obj = &(pal4a->lsc_func_obj);
    struct alLSCLib_set_parameter_t* lsc_set_para = &(pal4a->lsc_set_para);
    struct alLSCLib_get_parameter_t* lsc_get_para = &(pal4a->lsc_get_para);
    struct alLSClib_output_data_t* lsc_default_output_data = &(pal4a->lsc_default_output_data);

    lsc_err = lsc_func_obj->initial(lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    //lsc_set_para->type = SET_PARAM_TYPE_LSC_TUNING_BIN;
    //lsc_set_para->para.tuning_data = LSC_tuning_buf;
    //lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    //if (lsc_err != _AL_LSCLIB_SUCCESS) {
    //    return lsc_err;
    //}

    lsc_set_para->type = SET_PARAM_TYPE_LSC_COLOR_ORDER;
    lsc_set_para->para.color_order = TYPE_LSC_COLOR_ORDER_RGGB;
    lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    lsc_err = lsc_func_obj->calc_default_table(*lsc_obj, lsc_default_output_data);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }

    return lsc_err;
}

unsigned int lsc_run(al4ahd* pal4a)
{
    unsigned int lsc_err = _AL_LSCLIB_SUCCESS;

    void** lsc_obj = &(pal4a->lsc_obj);
    struct alLSClib_runtime_obj_t* lsc_func_obj = &(pal4a->lsc_func_obj);
    struct alLSCLib_set_parameter_t* lsc_set_para = &(pal4a->lsc_set_para);
    struct alLSClib_output_data_t* lsc_output = &(pal4a->lsc_output);

    //get ISO, AWB gain, BV, ParaIndex data from awb & ae
    lsc_set_para->type = SET_PARAM_TYPE_LSC_DYNAMIC_PROC_PARA;
    lsc_set_para->para.dynamic_proc_para.current_bv = 1000;
    lsc_set_para->para.dynamic_proc_para.iso_speed = 400;
    lsc_set_para->para.dynamic_proc_para.y_mean = 3825;
    lsc_set_para->para.dynamic_proc_para.interpolate_ct = 4000;
    lsc_err = lsc_func_obj->set_param(lsc_set_para, *lsc_obj);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }
    lsc_err = lsc_func_obj->dynamic_process(*lsc_obj, lsc_output);
    if (lsc_err != _AL_LSCLIB_SUCCESS) {
        return lsc_err;
    }
    *((volatile unsigned int *)0x20000034) = lsc_output.shading_table_00_address;
    return lsc_err;
}
