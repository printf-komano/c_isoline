#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


#include "src/isoline.c"


float f(il_vec2 arg, float * param, size_t param_len){
    return (sinf(arg[0]*param[1]) + sinf(arg[1]*param[1])) * param[0];
}


#define P_LEN 2

int main(){
    float * param = (float*) malloc( P_LEN * sizeof(float) );
    param[0] = 1.0f;
    param[1] = 4.0f;

    il_isoline_config cfg;
    cfg.offset[0] = 0.0f; cfg.offset[1] = 0.0f;
    cfg.scale[0] = 1.0f; cfg.scale[1] = 1.0f;
    cfg.f = &f;
    cfg.f_param = param;
    cfg.f_param_len = P_LEN;
    cfg.f_border_value = 1.0f;
    cfg.grid_len[0] = 10; cfg.grid_len[1] = 10;

    il_isoline_data data; 
    il_get_isoline_data(&data,cfg);


    return 0;
}
