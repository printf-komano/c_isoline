#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "ilutils.c"





typedef struct {
    il_vec2 vertex;
    size_t vertex_len;


    il_connection edges;
    size_t edges_len;
    
} il_pixel_solve_data;






#define GRID2_TO_FLAT(x, y, config) ((y) * (config).grid_len[0] + (x))

static inline void grid2_to_coordinates(
        size_t x, size_t y, il_isoline_config config, il_vec2 out
        )
{
    float x_grid_scale = (float)config.scale[0] / config.grid_len[0];
    float y_grid_scale = (float)config.scale[1] / config.grid_len[1];

    out[0] = config.offset[0] + x*x_grid_scale;
    out[1] = config.offset[1] + y*y_grid_scale;
}



static inline size_t add_vertex(il_pixel_solve_data * data, il_vec2 ver){
    for(size_t i=0; i<data->vertex_len; ++i){

        //merging
        if( 
                il_vec2_equal_approx(
                    data->vertex[i],
                    ver,
                    (float)config.scale[0] / config.grid_len[0]) * 0.1f
        ) return i;

        size_t offt = data->vertex_len;
        data->vertex[offt][0] = ver[0];
        data->vertex[offt][1] = ver[1];
        ++data->vertex_len;

        return offt;
    }
}


static inline void add_edge(il_pixel_solve_data * data, il_vec2 start, il_vec2 end){
    // decline if too short (merged points)
    if(
        il_vec2_equal_approx(
            start,
            end,
            (float)config.scale[0] / config.grid_len[0]) * 0.1f
        )
    ) return;
    
}



/*
if at least one value in 3x3 space is dofferent, return true
this chack may be high at const, but whatever
*/
static inline bool is_border_sample(il_vec2 dot, il_isoline_config con){
    il_vec2 dot_i;
    bool greater = false; bool ret = false;

    for (int i = 0; i < 9; ++i){
        // adding a half of the pixel on each iteration
        // grid is gonna be 3x3 (static size)
        float x_offt = (i%3) * (con.scale[0] / con.grid_len[0]) * 0.5f;
        float y_offt = (i/3) * (con.scale[1] / con.grid_len[1]) * 0.5f;

        dot_i[0]=dot[0]+x_offt; dot_i[1]=dot[1]+y_offt;

        float value = con.f( dot_i, con.f_param, con.f_param_len );

        // for the first iteration, we just check value
        // (less/greater)
        if(i==0) greater = value > con.f_border_value; 

        //on the next iterations, we look for the opposite 
        //(if there at least one value greater/less)
        else if(
                !greater && value > con.f_border_value ||
                greater && value < con.f_border_value
            ){
                return true;
        }
    }
    return false; 
}




void il_pixel_solve(il_isoline_config con, il_pixel_solve_data * data){

    /* samples are the points only on the border between
     * values higher than f_border_value or lower than one
     * sample placed only, if f() value is higher
    */
    il_vec2 * samples = (bool*) malloc(
            config.grid_len[0] * config.grid_len[1] * sizeof(il_vec2)
    );
    size_t samples_len = 0;

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


    


    free(samples);
}
