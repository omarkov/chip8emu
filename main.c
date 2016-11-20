#include <stdlib.h>

#include "SDL.h"

#include "chip8.h"


void update_screen(SDL_Surface *surface)
{
    //SDL_LockSurface(surface);
    
    u8 *vram = chip8_get_vram();
    u32 *pixels = (u32*)surface->pixels;

    for (int iy = 0; iy < 32; iy++)
        for (int cy = 0; cy < 10; cy++) 
            for (int ix = 0; ix < 64; ix++)
                for (int cx = 0; cx < 10; cx++)
                    *pixels++ = vram[iy * 64 + ix] * 0xFFFFFFFF;
    
    //SDL_UnlockSurface(surface);
    SDL_Flip(surface);
}

int main(int argc, char **argv)
{
    chip8_reset_state();
    chip8_load_rom("/Users/entrox/Coding/repos/chip8emu/chip8roms/syzygy");

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return 2;
    }
    
    atexit(SDL_Quit);
    
    // open a window
    SDL_Surface *screen = SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE);
    if (!screen) {
        printf("Unable to create window: %s\n", SDL_GetError());
        return 4;
    }
    
    double t0, t1;
    double frametime, tick_duration;
    
    // 60 hz
    tick_duration = 1000 / 20;
    
    t0 = SDL_GetTicks();
    
    while (1) {
        t1 = SDL_GetTicks();
        frametime = 0.0;
        
        // run a logic frame
        //while ((t1 - t0) > tick_duration) {
            chip8_execute_step();
            update_screen(screen);
            
            t0 += tick_duration;
            frametime += tick_duration;
            
            t1 = SDL_GetTicks();
       // }
        
        // event loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return 0;
                    
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE: return 0;
                        case SDLK_KP0: chip8_key_event(CHIP8_KEY_0, event.key.state); break;
                        case SDLK_KP1: chip8_key_event(CHIP8_KEY_7, event.key.state); break;
                        case SDLK_KP2: chip8_key_event(CHIP8_KEY_8, event.key.state); break;
                        case SDLK_KP3: chip8_key_event(CHIP8_KEY_9, event.key.state); break;
                        case SDLK_KP4: chip8_key_event(CHIP8_KEY_4, event.key.state); break;
                        case SDLK_KP5: chip8_key_event(CHIP8_KEY_5, event.key.state); break;
                        case SDLK_KP6: chip8_key_event(CHIP8_KEY_6, event.key.state); break;
                        case SDLK_KP7: chip8_key_event(CHIP8_KEY_1, event.key.state); break;
                        case SDLK_KP8: chip8_key_event(CHIP8_KEY_2, event.key.state); break;
                        case SDLK_KP9: chip8_key_event(CHIP8_KEY_3, event.key.state); break;

                        case SDLK_KP_EQUALS: chip8_key_event(CHIP8_KEY_A, event.key.state); break;
                        case SDLK_KP_DIVIDE: chip8_key_event(CHIP8_KEY_B, event.key.state); break;
                        case SDLK_KP_MULTIPLY: chip8_key_event(CHIP8_KEY_C, event.key.state); break;
                        case SDLK_KP_MINUS: chip8_key_event(CHIP8_KEY_D, event.key.state); break;
                        case SDLK_KP_PLUS: chip8_key_event(CHIP8_KEY_E, event.key.state); break;
                        case SDLK_KP_ENTER: chip8_key_event(CHIP8_KEY_F, event.key.state); break;
                        default: break;
                    }
                    break;
            }
        }
        
        // discard pending time
        if ((t1 - t0) > tick_duration)
            t0 = t1 - tick_duration;
    }
}


