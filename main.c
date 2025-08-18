#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>


#include "src/isoline.c"


float f(il_vec2 arg, float * param, size_t param_len){
    return (sinf(arg[0]*param[1]) + sqrtf(arg[1]*param[1])) * param[0];
}


#define P_LEN 3

#define LINES_COUNT 512
#define LINE_BORDER_START 0.00f
#define LINE_BORDER_DIFF 0.01f

float DRAW_SCALING = 10.0f;
float DRAW_OFFSET = 0.0f;


int main(){
    float * param = (float*) malloc( P_LEN * sizeof(float) );
    param[0] = 1.0f;
    param[1] = 16.0f;
    param[2] = 0.1f;

    il_isoline_config cfg;
    il_isoline_data * data = (il_isoline_data*)malloc(
            LINES_COUNT*sizeof(il_isoline_data)
    );


    for(size_t i=0; i<LINES_COUNT; ++i){
        cfg.offset[0] = 0.0f; cfg.offset[1] = 0.0f;
        cfg.scale[0] = 1.0f; cfg.scale[1] = 1.0f;
        cfg.f = &f;
        cfg.f_param = param;
        cfg.f_param_len = P_LEN;
        cfg.f_border_value = LINE_BORDER_START + LINE_BORDER_DIFF*i;
        cfg.grid_len[0] = 100; cfg.grid_len[1] = 100;

        il_get_isoline_data(&data[i],cfg);
    }

    



    // WINDOW 
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Hello SDL3!",
        1000, 1000, SDL_WINDOW_RESIZABLE);

    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }



    int running = 1;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = 0;
        }

        // Clear screen (black)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        
        for(size_t di=0; di<LINES_COUNT; ++di){
        il_isoline_data d = data[di];

        SDL_SetRenderDrawColor(renderer, 255, di*50, di*5, 255);


        for(size_t i=0; i<d.connections_len; ++i){
            il_connection ci;
            ci[0] = d.connections[i][0];
            ci[1] = d.connections[i][1];

            il_vec2i p0, p1;
            //coords 0
            p0[0] = d.points[ci[0]][0];
            p0[1] = d.points[ci[0]][1];
            
            //coords 1
            p1[0] = d.points[ci[1]][0];
            p1[1] = d.points[ci[1]][1];

            SDL_RenderLine(
                    renderer,
                    p0[0]*DRAW_SCALING + DRAW_OFFSET,
                    p0[1]*DRAW_SCALING + DRAW_OFFSET,
                    p1[0]*DRAW_SCALING + DRAW_OFFSET,
                    p1[1]*DRAW_SCALING + DRAW_OFFSET
            );
        }

        }
        //SDL_RenderLine(renderer, 0, 0, 300, 300); 
        // Present the result
        SDL_RenderPresent(renderer);
        

        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}






