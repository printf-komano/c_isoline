#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "ilutils.c"





typedef struct {
    il_vec2 offt;

    float (*f)(il_vec2, float *, size_t);   // any mathematical function 
                                            // that returns float value;
                                            // first 2 parameters are 
                                            // considered as (x,y)

    float * f_param;
    size_t f_param_len;
    float f_border_value; 


    float pixel_scale;
    il_vec2i pixel_len;

    size_t equ_iter;


} il_pixelsolve_config;







typedef struct {
    il_vec2 * vertex;
    size_t vertex_len;


    il_connection * edges;
    size_t edges_len;
    
} il_pixelsolve_data;









#define GRID2_TO_FLAT(x, y, con) ((y) * (con).pixel_len[0] + (x))

static inline void grid2_to_coordinates(
        size_t x, size_t y,
        il_pixelsolve_config con,
        il_vec2 out
        )
{

    out[0] = con.offt[0] + x*con.pixel_scale;
    out[1] = con.offt[1] + y*con.pixel_scale;
}




//adding vertices with auto merging
static inline size_t add_vertex(
        il_pixelsolve_data * data,
        il_vec2 ver,
        float merge_scale
        )
{
    for(size_t i=0; i<data->vertex_len; ++i){

        //if new vertex is close enough to old one, merge them
        if( 
                il_vec2_equal_approx(
                    data->vertex[i],
                    ver,
                    merge_scale
                )
                    
        ) return i;


        size_t offt = data->vertex_len;
        data->vertex[offt][0] = ver[0];
        data->vertex[offt][1] = ver[1];
        ++data->vertex_len;

        return offt;
    }
}



static inline int32_t add_edge(
        il_pixelsolve_data * data,
        il_vec2 start,
        il_vec2 end,
        float merge_scale
        )
{
    // decline if too short (merged points)
    if(
        il_vec2_equal_approx(start,end,merge_scale)
    ) return -1;


    size_t ind0 = add_vertex(data,start,merge_scale);
    size_t ind1 = add_vertex(data,end,merge_scale);

    size_t offt = data->edges_len;
    data->edges[offt][0] = ind0;
    data->edges[offt][1] = ind1;
    ++data->edges_len;
    return offt;
}





/*
if at least one value in 3x3 space is dofferent, return true
this chack may be high at const, but whatever
*/
static inline bool is_border_sample(il_vec2 dot, il_pixelsolve_config con){
    il_vec2 dot_i;
    bool greater = false; bool ret = false;

    for (int i = 0; i < 9; ++i){
        // adding a half of the pixel on each iteration
        // grid is gonna be 3x3 (static size)
        float x_offt = (i%3) * con.pixel_scale * 0.5f;
        float y_offt = (i/3) * con.pixel_scale * 0.5f;

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




static inline size_t pixel_solve_equ(
        il_vec2 dot,            // starting point
        il_vec2 direction,      // 1-vector, direction (horizontal/vertical)
        il_vec2 out,            // RESULT
        il_pixelsolve_config con
        )
{
    size_t solutions = 0;

    // summary local offsed of the found solutions
    // (later we well divide it by number to calc average coords)
    il_vec2 sum_offt;
    sum_offt[0] = 0.0f;
    sum_offt[1] = 0.0f;

    // select a starting point
    il_vec2 dot_i; dot_i[0] = dot[0]; dot_i[1] = dot[1];
    float iter_scale = con.pixel_scale / (float)con.equ_iter;


    //get value at starting point (to compare later)
    float val;
    float prev_val = con.f(
            dot_i,
            con.f_param,
            con.f_param_len
    );

    for(size_t i=1; i<con.equ_iter; ++i){

        // move towards direction 
        dot_i[0] += direction[0] * iter_scale; 
        dot_i[1] += direction[1] * iter_scale;

        val = con.f(
                dot_i,
                con.f_param,
                con.f_param_len
        );
        
        // if we see value crossing f_border_value,
        // that means we met intersection point.
        // add the answer to average result
        if(
            (val > con.f_border_value && prev_val < con.f_border_value) ||
            (val < con.f_border_value && prev_val > con.f_border_value)

        ){
            ++solutions; // increase number of sol
            prev_val = val; // now they are equal till next change
            
            // shift local coords
            sum_offt[0] += dot_i[0] - dot[0];
            sum_offt[1] += dot_i[1] - dot[1];
        }
        
        // return result
        out[0] = dot[0] + sum_offt[0] / (float)solutions;
        out[1] = dot[1] + sum_offt[1] / (float)solutions;

    }

    return solutions;
}







static inline void pixel_solve(
        il_pixelsolve_data * data,
        il_vec2 dot,
        il_pixelsolve_config con
        )
{

    // two opposite corners 

    il_vec2 dot0;
    il_vec2 dot1;

    dot0[0] = dot[0];
    dot0[1] = dot[1];
    dot1[0] = dot[0] + con.pixel_scale;
    dot1[1] = dot[1] + con.pixel_scale;



    // directions for solving equations
    il_vec2 dir[4] = {
        {1.0f,  0.0f},
        {0.0f,  1.0f},
        {-1.0f, 0.0f},
        {0.0f,  -1.0f}
    };

    il_vec2 res[4];
    size_t res_len[4];
    size_t vertex_ind[4];

    // solving two equations from opposite corners 
    // (to cover a full pixel contour)

    res_len[0] = pixel_solve_equ(dot0,dir[0],res[0],con);
    res_len[1] = pixel_solve_equ(dot0,dir[1],res[1],con);

    res_len[2] = pixel_solve_equ(dot1,dir[2],res[2],con);
    res_len[2] = pixel_solve_equ(dot1,dir[3],res[3],con);

    if(
        res_len[0]+res_len[1]+res_len[2]+res_len[3] < 2 
    ) return;
    
    float prec = con.pixel_scale / con.equ_iter; 


    if( res_len[0]>0 && res_len[1]>0 ) add_edge(data, res[0], res[1], prec);
    if( res_len[1]>0 && res_len[2]>0 ) add_edge(data, res[1], res[2], prec);
    if( res_len[2]>0 && res_len[3]>0 ) add_edge(data, res[2], res[3], prec);
    if( res_len[3]>0 && res_len[1]>0 ) add_edge(data, res[3], res[0], prec);
    

}






void il_pixelsolve_isoline(il_pixelsolve_config con, il_pixelsolve_data * data){

    /* samples are the points only on the border between
     * values higher than f_border_value or lower than one
     * sample placed only, if f() value is higher
    */
    il_vec2 * samples = (il_vec2*) malloc(
            con.pixel_len[0] * con.pixel_len[1] * sizeof(il_vec2)
    );
    size_t samples_len = 0;

    free(samples);
}
