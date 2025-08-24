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









//#define GRID2_TO_FLAT(x, y, con) ((y) * (con).pixel_len[0] + (x))

/*static inline void grid2_to_coordinates(
        size_t x, size_t y,
        il_pixelsolve_config con,
        il_vec2 out
        )
{

    out[0] = con.offt[0] + x*con.pixel_scale;
    out[1] = con.offt[1] + y*con.pixel_scale;
}
*/



//adding vertices with auto merging
static inline size_t add_vertex(
        il_pixelsolve_data * data,
        il_vec2 ver,
        float merge_scale
        )
{
    /*printf("adding vert at:\t(%f:%f);\n",
            ver[0],ver[1]
    );*/

    
    for(size_t i=0; i<data->vertex_len; ++i){

        //if new vertex is close enough to old one, merge them
        if( 
                il_vec2_equal_approx(
                    data->vertex[i],
                    ver,
                    merge_scale
                )
                    
        ){
            //printf("\t\tnew vert MRG with %d\n", i);
            return i;
        } 
    }
    


    //increase the number of vertex (added new one)
    size_t offt = data->vertex_len;
    data->vertex[offt][0] = ver[0];
    data->vertex[offt][1] = ver[1];
    ++data->vertex_len;
    //printf("\t\tnew vert ADD with %d\n", offt);
    return offt;
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

    if ( ind0 == ind1 ) return -2;

    size_t offt = data->edges_len;
    data->edges[offt][0] = ind0;
    data->edges[offt][1] = ind1;
    ++data->edges_len;

    /*printf("added edge %d(%d - %d) at:\t(%f:%f)\t(%f:%f);\n",
            offt,
            ind0,
            ind1,
            start[0],start[1],
            end[0],end[1]
    );*/

    return offt;
}





/*
this functions checks 4 corners of a pixel.
if one of the values is different (oppisite of f_border_value),
return true.
*/
static inline bool is_border_sample(il_vec2 dot, il_pixelsolve_config con){
    il_vec2 corners[4];
    float first;


    /*
        0--1
        |  |
        2--3
    */
    corners[0][0] = dot[0]; 
    corners[0][1] = dot[1];

    corners[1][0] = dot[0] + con.pixel_scale;
    corners[1][1] = dot[1];

    corners[2][0] = dot[0];
    corners[2][1] = dot[1] + con.pixel_scale;

    corners[3][0] = dot[0] + con.pixel_scale;
    corners[3][1] = dot[1] + con.pixel_scale;



    for(size_t i=0; i<4; ++i){
        float fi = con.f(corners[i], con.f_param, con.f_param_len);
        if(i==0) first = fi;
        else if (
                fi > con.f_border_value && first < con.f_border_value ||
                fi < con.f_border_value && first > con.f_border_value
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
    float iter_scale = con.pixel_scale / (float)con.equ_iter;

    // select a starting point
    il_vec2 dot_i; 
    dot_i[0] = dot[0]; dot_i[1] = dot[1];


    /*printf("\tequ(prec %f):\tstart(%f:%f)\tdir(%f:%f)\n",
        iter_scale,
        dot_i[0],dot_i[1],
        direction[0],direction[1]
    );*/


    // summary local offsed of the found solutions
    // (later we well divide it by number to calc average coords)
    il_vec2 sum_offt;
    sum_offt[0] = 0.0f;
    sum_offt[1] = 0.0f;

    

    //get value at starting point (to compare later)
    float val = con.f(
            dot_i,
            con.f_param,
            con.f_param_len
    );
    float prev_val = val;

    for(size_t i=1; i<con.equ_iter+1; ++i){

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
            
            /*
                //attemt debug
                out[0] = dot_i[0];
                out[1] = dot_i[1];
                return 1;
            */



            // shift local coords
            sum_offt[0] += dot_i[0] - dot[0];
            sum_offt[1] += dot_i[1] - dot[1];
        }    
    }

    // return result
    if(solutions==0) return 0;

    out[0] = dot[0] + (sum_offt[0] / (float)solutions);
    out[1] = dot[1] + (sum_offt[1] / (float)solutions);


    /*printf("\tequ solved:\t%f:%f\n",
        out[0],
        out[1]
    );*/

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

    //printf("CORNER0:\t%f:%f\n",dot0[0],dot0[1]);

    dot1[0] = dot0[0] + con.pixel_scale;
    dot1[1] = dot0[1] + con.pixel_scale;

    //printf("CORNER1:\t%f:%f\n",dot1[0],dot1[1]);




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
    res_len[3] = pixel_solve_equ(dot1,dir[3],res[3],con);

    
    if(
        res_len[0]+res_len[1]+res_len[2]+res_len[3] < 2 
    ) return;

    float prec = con.pixel_scale / (float)con.equ_iter; 


    if( res_len[0]>0 && res_len[1]>0 ) add_edge(data, res[0], res[1], 2.0f*prec);
    if( res_len[1]>0 && res_len[2]>0 ) add_edge(data, res[1], res[2], 2.0f*prec);
    if( res_len[2]>0 && res_len[3]>0 ) add_edge(data, res[2], res[3], 2.0f*prec);
    if( res_len[3]>0 && res_len[0]>0 ) add_edge(data, res[3], res[0], 2.0f*prec);
    





    if( 
            res_len[0]>0 && res_len[2]>0 &&
            (res_len[1]==0 && res_len[3]==0)
    ) add_edge(data, res[0], res[2], 2.0f*prec);

    if( 
            res_len[1]>0 && res_len[3]>0 &&
            (res_len[0]==0 && res_len[2]==0)
    ) add_edge(data, res[1], res[3], 2.0f*prec);



    //if( res_len[0]>0 && res_len[2]>0 ) add_edge(data, res[0], res[2], 2.0f*prec);

}






void il_pixelsolve_isoline(il_pixelsolve_data * data, il_pixelsolve_config con){
    size_t max_samples = con.pixel_len[0] * con.pixel_len[1];

    data->vertex = (il_vec2*) malloc(
            max_samples * 8 * sizeof(il_vec2)
    );
    data->vertex_len = 0;

    data->edges = (il_connection*) malloc(
            max_samples * 8 * sizeof(il_connection)
    );
    data->edges_len = 0;




    il_vec2 samplei;

    for(size_t x=0;x<con.pixel_len[0];++x){
        for(size_t y=0;y<con.pixel_len[1];++y){
            samplei[0] = con.offt[0] + x*con.pixel_scale;
            samplei[1] = con.offt[1] + y*con.pixel_scale;

            if(!is_border_sample(samplei,con)) continue;
            pixel_solve(data,samplei,con);
        }
    }

}
