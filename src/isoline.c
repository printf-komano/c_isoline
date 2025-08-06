#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

/*
The main goal is to trace a contour (line)
where function f(x,y) has the certain value

we can start from type definitions
*/

typedef float il_vec2[2];
typedef uint32_t il_vec2i[2];
typedef uint32_t il_connection[2];


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



static void push_point(il_vec2 point){
        
}

static void push_connection(il_connection connection){

}



static float directed_difference(
        float (*f)(il_vec2, float*, size_t),
        float * param,
        size_t param_len,
        il_vec2 pos0,
        il_vec2 pos1
        ) 
{
    
    float value0 = f(pos0,param,param_len);
    float value1 = f(pos1,param,param_len);

    return value0 - value1; //difference between points
}





static inline size_t grid2_to_flat(size_t x, size_t y, size_t x_len){
    return x + y*x_len;
}


#define GRID2_TO_FLAT(x,y,config) (x + y*config.grid_len[0])

static inline void grid2_to_coordinates(
        size_t x, size_t y, il_isoline_config config, il_vec2 out
        )
{
    float x_grid_scale = (float)config.scale[0] / config.grid_len[0];
    float y_grid_scale = (float)config.scale[1] / config.grid_len[1];

    out[0] = config.offset[0] + x*x_grid_scale;
    out[1] = config.offset[1] + y*y_grid_scale;
}





static inline bool is_border_value(
        size_t x, size_t y,
        float * f_values,
        il_isoline_config config
        )
{
    bool in_border = 
        f_values[GRID2_TO_FLAT(x,y,config)] >= config.f_border_value;
    if(!in_border) return false;
    
    //may be on the edge of the grid itself
    if(
            x <= 0 ||
            y <= 0 ||
            x >= (config.grid_len[0]-1) ||
            y >= (config.grid_len[1]-1)
    ){
        //printf("value on edge.\n");
        return true;
    }
    
    //printf("value matches conditions and not on the edge\n");
    return (
        f_values[GRID2_TO_FLAT(x+1,y,config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x-1,y,config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x,y+1,config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x,y-1,config)] < config.f_border_value
    );
    
    

}


void il_get_isoline_data(il_isoline_data * out, il_isoline_config config){
    
    //first step is to create value grid, where
    float * f_values = (float*) malloc(
            config.grid_len[0] * config.grid_len[1] * sizeof(float)
    );

    bool * border_points = (bool*) malloc(
            config.grid_len[0] * config.grid_len[1] * sizeof(bool)
    );


    //get the copy of parameters (will be modified)
    float * f_param = (float*) malloc(config.f_param_len*sizeof(float));
    memcpy(f_param, config.f_param, config.f_param_len*sizeof(float));

    //calculating all values for each x,y: f(x,y)
    for (size_t yi=0; yi<config.grid_len[1]; ++yi){
        for (size_t xi=0; xi<config.grid_len[0]; ++xi){
            il_vec2 point_i;

            //get x,y pair for f(x,y)
            grid2_to_coordinates(xi,yi,config,point_i);
            float f_i = config.f( point_i, f_param, config.f_param_len );
            
            //calculate value
            f_values[ GRID2_TO_FLAT(xi,yi,config) ] = f_i;
        }
    }



    //select border points
    for (size_t yi=0; yi<config.grid_len[1]; ++yi){
        for (size_t xi=0; xi<config.grid_len[0]; ++xi){
            il_vec2 point_i;
            bool bvalue = is_border_value(xi,yi, f_values, config);
            border_points[ GRID2_TO_FLAT(xi,yi,config) ] = bvalue;
            
            if(bvalue) printf("o ");
            else printf(". ");
        }
        printf("\n");
    }

    






}
