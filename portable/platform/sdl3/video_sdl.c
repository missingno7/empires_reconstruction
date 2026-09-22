/* video_sdl.c -- SDL3 window/renderer/texture wrapper (see video_sdl.h). */
#include "video_sdl.h"

#include <SDL3/SDL.h>

static SDL_Window   *s_window   = NULL;
static SDL_Renderer *s_renderer = NULL;
static SDL_Texture  *s_texture  = NULL;
static bool           s_quit_requested = false;

bool sdl_video_init(const char *title)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    s_window = SDL_CreateWindow(title,
                                 SDL_VIDEO_LOGICAL_W * 2, SDL_VIDEO_LOGICAL_H * 2,
                                 SDL_WINDOW_RESIZABLE);
    if (!s_window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }

    s_renderer = SDL_CreateRenderer(s_window, NULL);
    if (!s_renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return false;
    }

    if (!SDL_SetRenderLogicalPresentation(s_renderer,
                                           SDL_VIDEO_LOGICAL_W, SDL_VIDEO_LOGICAL_H,
                                           SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        SDL_Log("SDL_SetRenderLogicalPresentation failed: %s", SDL_GetError());
        return false;
    }

    s_texture = SDL_CreateTexture(s_renderer, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SDL_VIDEO_LOGICAL_W, SDL_VIDEO_LOGICAL_H);
    if (!s_texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetTextureScaleMode(s_texture, SDL_SCALEMODE_NEAREST);

    return true;
}

void sdl_video_shutdown(void)
{
    if (s_texture) {
        SDL_DestroyTexture(s_texture);
        s_texture = NULL;
    }
    if (s_renderer) {
        SDL_DestroyRenderer(s_renderer);
        s_renderer = NULL;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = NULL;
    }
    SDL_Quit();
}

void sdl_video_present(const uint8_t *vram8, const uint8_t *dac6)
{
    if (!s_renderer || !s_texture) {
        return;
    }

    void *pixels = NULL;
    int pitch = 0;
    if (!SDL_LockTexture(s_texture, NULL, &pixels, &pitch)) {
        SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
        return;
    }

    for (int y = 0; y < SDL_VIDEO_LOGICAL_H; ++y) {
        uint32_t *dst_row = (uint32_t *)((uint8_t *)pixels + (size_t)y * (size_t)pitch);
        const uint8_t *src_row = vram8 + (size_t)y * SDL_VIDEO_LOGICAL_W;
        for (int x = 0; x < SDL_VIDEO_LOGICAL_W; ++x) {
            uint8_t index = src_row[x];
            const uint8_t *rgb6 = dac6 + (size_t)index * 3;
            /* 6-bit DAC channel (0..63) -> 8-bit channel: v<<2 | v>>4. */
            uint8_t r = (uint8_t)((rgb6[0] << 2) | (rgb6[0] >> 4));
            uint8_t g = (uint8_t)((rgb6[1] << 2) | (rgb6[1] >> 4));
            uint8_t b = (uint8_t)((rgb6[2] << 2) | (rgb6[2] >> 4));
            dst_row[x] = ((uint32_t)0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        }
    }

    SDL_UnlockTexture(s_texture);

    SDL_RenderClear(s_renderer);
    SDL_RenderTexture(s_renderer, s_texture, NULL, NULL);
    SDL_RenderPresent(s_renderer);
}

void sdl_video_toggle_fullscreen(void)
{
    if (!s_window)
        return;
    bool full = (SDL_GetWindowFlags(s_window) & SDL_WINDOW_FULLSCREEN) != 0;
    SDL_SetWindowFullscreen(s_window, !full);
}

bool sdl_video_poll_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            s_quit_requested = true;
        } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            s_quit_requested = true;
        }
    }
    return s_quit_requested;
}
