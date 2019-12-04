#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

///////////////////////////////////////////////////////////////////////////////
// Define constants for FPS and game loop frame time
///////////////////////////////////////////////////////////////////////////////
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

///////////////////////////////////////////////////////////////////////////////
// Define global variables to handle SDL window and renderer
///////////////////////////////////////////////////////////////////////////////
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

unsigned window_width = 800;
unsigned window_height = 600;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture;

///////////////////////////////////////////////////////////////////////////////
// Function to initialize the SDL Window and Renderer
///////////////////////////////////////////////////////////////////////////////
int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Set width and height of the SDL Window with the max screen resolution
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Set a pixel with a given colour
///////////////////////////////////////////////////////////////////////////////
void draw_pixel(int x, int y, uint32_t color) {
    color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Render a line using the DDA line drawing algorithm
///////////////////////////////////////////////////////////////////////////////
void draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int delta_x = (x2 - x1);
    int delta_y = (y2 - y1);

    int step = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // DDA is naive and slow as it uses floating point for step increment
    float x_inc = delta_x / (float) step;
    float y_inc = delta_y / (float) step;

    float x = x1;
    float y = y1;
    for (int i = 0; i <= step; i++) {
        draw_pixel((int)x, (int)y, color);
        x += x_inc;
        y += y_inc;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Draw an unfilled triangle using raw line calls
///////////////////////////////////////////////////////////////////////////////
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

///////////////////////////////////////////////////////////////////////////////
// Renders the color buffer array in a texture and displays it
///////////////////////////////////////////////////////////////////////////////
void render_color_buffer() {
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)((uint32_t)window_width * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Routine to clear the entire color buffer with a single color value
///////////////////////////////////////////////////////////////////////////////
void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++)
        for (int x = 0; x < window_width; x++)
            if (x % 5 == 0 && y % 5 == 0)
                color_buffer[(window_width * y) + x] = 0xFF333333;
            else
                color_buffer[(window_width * y) + x] = color;
}

///////////////////////////////////////////////////////////////////////////////
// Function to destroy renderer, window, and exit SDL
///////////////////////////////////////////////////////////////////////////////
void destroy_window(void) {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif
