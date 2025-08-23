#pragma once

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

/*
The main goal is to trace a contour (line)
where function f(x,y) has the certain value

we can start from type definitions
*/

typedef float il_vec2[2];
typedef uint32_t il_vec2i[2];
typedef uint32_t il_connection[2];
typedef float il_section[4];



bool il_vec2_equal_approx(il_vec2 a, il_vec2 b, float prec) {
    return (
        prec > 0.0f &&
        
        fabsf(a[0] - b[0]) <= prec &&
        fabsf(a[1] - b[1]) <= prec
    );
}


/*
inital cofing for building isoline.
*/
typedef struct {
    il_vec2 offset; // starting position for calculations
    il_vec2 scale; // size of the area


    float (*f)(il_vec2, float *, size_t);   // any mathematical function 
                                            // that returns float value;
                                            // first 2 parameters are 
                                            // considered as (x,y)

    float * f_param;
    size_t f_param_len;

    float f_border_value; // value where line is going
    
    il_vec2i grid_len;
        
} il_isoline_config;






