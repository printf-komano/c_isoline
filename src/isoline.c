#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>


/*
The main goal is to trace a contour (line)
where function f(x,y) has the certain value

we can start from type definitions
*/

typedef float il_vec2[2];
typedef uint32_t il_connection[2];


/*
inital cofing for building isoline.
*/
typedef struct {
    il_vec2 offset; // starting position for calculations
    il_vec2 scale; // size of the area

    
    float (*f)(float *, size_t);    // any mathematical function 
                                    // that returns float value;
                                    // first 2 parameters are 
                                    // considered as (x,y)
    size_t f_param_len;

    float f_border_value; // value where line is going
    
} il_isoline_config;




/*
data of the result isoline
*/
typedef struct {
    il_vec2 * points;
    size_t points_len;
    size_t points_cap;

    il_connection * connections;
    size_t connections_len;
    size_t connections_cap;
} il_isoline_data;




static float directed_difference(
        float (*f)(float*, size_t),
        float * param,
        size_t param_len,
        il_vec2 direction
        ) 
{
    
    float value_start = f(param,param_len);

    float * param_end = (float*) malloc(param_len*sizeof(float));
    memcpy(param_end, param, param_len*sizeof(float));

    float value_end = f(param_end,param_len);

    return value_end - value_start; //difference between points
}




void il_get_isoline_data(il_isoline_data * out, il_isoline_config * config);







void il_get_isoline_data(il_isoline_data * out, il_isoline_config * config){

}
