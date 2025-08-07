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
    il_vec2i * points;
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


static bool connection_exists(il_isoline_data data, size_t index0, size_t index1){
    if(index0 == index1) return true; //connection to itself

    //il_vec2i p = points[index];
    for(size_t i=0; i<data.connections_len; ++i){
        if(
            (data.connections[i][0]==index0 && data.connections[i][1]==index1) ||
            (data.connections[i][0]==index1 && data.connections[i][1]==index0)
        ){
            return true;
        }
    }
    return false;
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





static inline bool is_on_border(
        size_t x, size_t y,
        float * f_values,
        il_isoline_config config
        )
{
    float len_y = config.grid_len[1],
    len_x = config.grid_len[0];


    bool greater = 
        f_values[GRID2_TO_FLAT(x,y,config)] >= config.f_border_value;

    if(!greater) return false;
    
    //may be on the edge of the grid itself
    if(
            x == 0 ||
            y == 0 ||
            x >= (config.grid_len[0]-1) ||
            y >= (config.grid_len[1]-1)
    ){
        //printf("value on edge.\n");
        return true;
    }
    
    //printf("value matches conditions and not on the edge\n");
    return (
        f_values[GRID2_TO_FLAT(x+1, y, config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x-1, y, config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x, y+1, config)] < config.f_border_value ||
        f_values[GRID2_TO_FLAT(x, y-1, config)] < config.f_border_value
    );
    
    

}


void il_get_isoline_data(il_isoline_data * data, il_isoline_config config){
    
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


    data->points_len = 0;
    //select border points
    for (size_t yi=0; yi<config.grid_len[1]; ++yi){
        for (size_t xi=0; xi<config.grid_len[0]; ++xi){
            il_vec2 point_i;
            bool bvalue = is_on_border(xi,yi, f_values, config);
            border_points[ GRID2_TO_FLAT(xi,yi,config) ] = bvalue;
            
            data->points_len += bvalue;

            if(bvalue) printf("o");
            else printf(".");
        }
        printf("\n");
    }

    data->points = (il_vec2i*) malloc( data->points_len*sizeof(il_vec2i) );
    size_t point_count = 0;
    //il_vec2 pi;
    //grid2_to_coordinates(0,0,config,pi);
    
    for (size_t yi=0; yi<config.grid_len[1]; ++yi){
        for (size_t xi=0; xi<config.grid_len[0]; ++xi){
            if(border_points[GRID2_TO_FLAT(xi,yi,config)]){
                il_vec2i pi;
                //grid2_to_coordinates(xi,yi,config,pi);
                //printf("%d added point at cords %d,%d\n",
                //    point_count,xi,yi        
                //);

                data->points[point_count][0] = xi;
                data->points[point_count][1] = yi;
                
                ++point_count;
            }
        }
    }

    //printf("\nbuilding connections\n");

    //building connections 
    
    data->connections_cap = config.grid_len[0] * config.grid_len[1] * 2;
    data->connections = (il_connection*) 
        malloc(data->connections_cap * sizeof(il_connection));
    data->connections_len = 0;
    
    for(size_t i=0; i<data->points_len; ++i){
        for(size_t j=0; j<data->points_len; ++j){
            if(connection_exists(*data,i,j)) continue;

            //check distance between points
            float dx = abs( data->points[i][0] - data->points[j][0] );
            float dy = abs( data->points[i][1] - data->points[j][1] );
            if(dx > 1.0f || dy > 1.0f) continue;

            //add new connection
            data->connections[data->connections_len][0] = i;
            data->connections[data->connections_len][1] = j;

            //printf("\t%d, %d-%d\n",data->connections_len,i,j);
            ++data->connections_len;
        }   
    }

    




    


    
    




}
