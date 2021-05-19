/*
 * Emulator for the UEFI GOP (Graphics Output Protocol) using SDL2
 */

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <efi.h>

// We hardcode window attributes for now
#define GOPEMU_WIDTH 800
#define GOPEMU_HEIGHT 600

static efi_graphics_output_mode_information gopemu_mode_info = {
    .version = 0,
    .horizontal_resolution = GOPEMU_WIDTH,
    .vertical_resolution = GOPEMU_HEIGHT,
    .pixel_format = pixel_red_green_blue_reserved_8_bit_per_color,
    .pixel_information = {
        .red_mask      = 0xff000000,
        .green_mask    = 0x00ff0000,
        .blue_mask     = 0x0000ff00,
        .reserved_mask = 0x000000ff,
    },
    .pixels_per_scan_line = GOPEMU_WIDTH,
};

static efi_u32 gopemu_fbdata[GOPEMU_WIDTH * GOPEMU_HEIGHT];

static efi_graphics_output_protocol_mode gopemu_mode = {
    .max_mode = 0,
    .mode = 0,
    .info = &gopemu_mode_info,
    .size_of_info = sizeof gopemu_mode_info,
    .frame_buffer_base = (efi_physical_address) gopemu_fbdata,
    .frame_buffer_size = sizeof gopemu_fbdata,
};

// We setup SDL globally using C++ style global constructors
static void __attribute__((constructor)) init_sdl(void)
{
    SDL_Init(SDL_INIT_VIDEO);
}

static void __attribute__((destructor)) deinit_sdl(void)
{
    SDL_Quit();
}

typedef struct __attribute__((packed)) {
    // We provide GOP functions
    efi_graphics_output_protocol gop;
    // And than put our internal fields here
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *fb;
    // FIXME: this should be locked or atomic, not volatile
    volatile _Bool keep_running;
    // We run the SDL event loop in this thread
    pthread_t loop_thread;
} gopemu;

static void *gopemu_update(void *userdata)
{
    gopemu *self = userdata;
    while (self->keep_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event))
            switch (event.type) {
            case SDL_QUIT:
                kill(getpid(), SIGINT);
                goto end;
            }

        SDL_UpdateTexture(self->fb, NULL,
                          (void *) self->gop.mode->frame_buffer_base,
                          sizeof(efi_u32) *
                          self->gop.mode->info->pixels_per_scan_line);
        SDL_RenderClear(self->renderer);
        SDL_RenderCopy(self->renderer, self->fb, NULL, NULL);
        SDL_RenderPresent(self->renderer);
    }
end:
    return NULL;
}

efi_graphics_output_protocol *gopemu_init(void)
{
    gopemu *self = malloc(sizeof *self);
    self->window = SDL_CreateWindow("UEFI Graphics Output",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    GOPEMU_WIDTH,
                                    GOPEMU_HEIGHT,
                                    0);
    self->renderer = SDL_CreateRenderer(self->window, -1, 0);
    // Alpha is close enough to reserved, right? :')
    self->fb = SDL_CreateTexture(self->renderer,
                                 SDL_PIXELFORMAT_ABGR8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 GOPEMU_WIDTH, GOPEMU_HEIGHT);

    // Provide UEFI framebuffer using a texture
    self->gop.mode = &gopemu_mode;


    // Event loop thread
    self->keep_running = 1;
    pthread_create(&self->loop_thread, NULL, gopemu_update, self);
    return &self->gop;
}

void gopemu_deinit(efi_graphics_output_protocol *gop)
{
    gopemu *self = (gopemu *) gop;

    // Wait for event loop to exit
    self->keep_running = 0;
    pthread_join(self->loop_thread, NULL);

    // Destroy window
    SDL_DestroyTexture(self->fb);
    SDL_DestroyRenderer(self->renderer);
    SDL_DestroyWindow(self->window);
    free(self);
}
