#include <stdio.h>
#include <math.h>
#include <stdint.h>



/*
The main goal is to trace a contour (line)
where function f(x,y) has the certain value

we can start from type definitions
*/

typedef float il_vec2[2];
typedef uint32_t il_connection[2];



typedef struct {
    il_vec2 scale;
    il_vec2 offset;
} il_isoline_config;





typedef struct {
    il_vec2 * points;
    il_connection * connections;
} il_isoline_description;



