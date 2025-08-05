#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>


#include "src/isoline.c"


float f(float * param, size_t param_len){
    return (param[0] + param[1]) * param[2];
}


#define P_LEN 3

int main(){
    float * param = (float*) malloc( P_LEN * sizeof(float) );
    param[2] = 0.01f;

    il_isoline_config cfg;
    cfg.offset[0] = 0.0f; cfg.offset[1] = 0.0f;
    cfg.scale[1] = 1.0f; cfg.scale[1] = 1.0f;
    cfg.f = &f;
    cfg.f_param = param;
    cfg.f_param_len = P_LEN;
    cfg.f_border_value = 0.5f;
    cfg.grid_len[0] = 10; cfg.grid_len[1] = 10;

    il_isoline_data data; 
    il_get_isoline_data(&data,cfg);


    return 0;
}
