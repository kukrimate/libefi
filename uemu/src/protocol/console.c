//
// Emulator for the UEFI console using SDL2, it implements the following protocols:
//  * EFI Graphics Output Protocol (GOP)
//  * EFI Simple Text Input Protocol
//  * EFI Simple Text Output Protocol
//

#include <efi.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>
#include "console.h"
#include "util.h"

//
// Make sure there always is a global SDL context, this lets us avoid some
// housekeeping when creating/exiting console instances
//
static void __attribute__((constructor)) init_sdl(void)
{
    SDL_Init(SDL_INIT_VIDEO);
}

static void __attribute__((destructor)) deinit_sdl(void)
{
    SDL_Quit();
}

//
// Window size
//
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

struct ConsoleHandle {
    // Exit callback
    ConsoleExitCallback exit_callback;

    // And than put our internal fields here
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    // We run the SDL event loop in this thread
    atomic_bool keep_running;
    pthread_t loop_thread;

    // Input queue
    //
    // FIXME: this is not really a queue for now, it returns the latest
    // keypress first instead of the oldest one in the queue
    //
    pthread_mutex_t input_queue_lock; // Only one thread can touch the queue
    size_t input_queue_count;         // Number of queued keys
    efi_in_key input_queue[16];       // Array of queued keys

    // Supported EFI protocols
    efi_graphics_output_protocol gop;
    efi_simple_text_in_protocol text_in;
    efi_simple_text_out_protocol text_out;
};

static efi_status efiapi text_in_reset(efi_simple_text_in_protocol *this,
    efi_bool ext_verf)
{
    ConsoleHandle *self = (void *) this - offsetof(ConsoleHandle, text_in);

    // Empty input queue (per UEFI specification)
    pthread_mutex_lock(&self->input_queue_lock);
    self->input_queue_count = 0;
    pthread_mutex_unlock(&self->input_queue_lock);

    return EFI_SUCCESS;
}

static efi_status efiapi text_in_read_key(efi_simple_text_in_protocol *this,
    efi_in_key *key)
{
    ConsoleHandle *self = (void *) this - offsetof(ConsoleHandle, text_in);

    // Return EFI_NOT_READY if the queue is empty
    pthread_mutex_lock(&self->input_queue_lock);
    if (self->input_queue_count == 0) {
        pthread_mutex_unlock(&self->input_queue_lock);
        return EFI_NOT_READY;
    }

    // Copy key from input queue
    *key = self->input_queue[--self->input_queue_count];
    pthread_mutex_unlock(&self->input_queue_lock);

    return EFI_SUCCESS;
}

//
// Translate an SDL keyboard event to an efi_in_key
//
static void translate_scancode(ConsoleHandle *self, SDL_KeyboardEvent *sdl_key)
{
    pthread_mutex_lock(&self->input_queue_lock);

    // Only translate event if it fits in the queue
    if (self->input_queue_count == ARRAY_SIZE(self->input_queue))
        goto out;

    efi_in_key key = {
        .scan = 0,
        .c = 0
    };

    // Translate scancode
    switch (sdl_key->keysym.scancode) {
    // SDL scancodes that map to unicode control chars in UEFI
    case SDL_SCANCODE_BACKSPACE: key.c = CHAR_BACKSPACE; break;
    case SDL_SCANCODE_TAB:       key.c = CHAR_TAB; break;
    // NOTE: EDK2 produces this charcode for return too
    case SDL_SCANCODE_RETURN:    key.c = CHAR_CARRIAGE_RETURN; break;
    // SDL scancodes that have a UEFI mapping
    case SDL_SCANCODE_UP:        key.scan = SCAN_UP; break;
    case SDL_SCANCODE_DOWN:      key.scan = SCAN_DOWN; break;
    case SDL_SCANCODE_LEFT:      key.scan = SCAN_LEFT; break;
    case SDL_SCANCODE_RIGHT:     key.scan = SCAN_RIGHT; break;
    case SDL_SCANCODE_HOME:      key.scan = SCAN_HOME; break;
    case SDL_SCANCODE_END:       key.scan = SCAN_END; break;
    case SDL_SCANCODE_INSERT:    key.scan = SCAN_INSERT; break;
    case SDL_SCANCODE_DELETE:    key.scan = SCAN_DELETE; break;
    case SDL_SCANCODE_PAGEUP:    key.scan = SCAN_PAGE_UP; break;
    case SDL_SCANCODE_PAGEDOWN:  key.scan = SCAN_PAGE_DOWN; break;
    case SDL_SCANCODE_F1:        key.scan = SCAN_F1; break;
    case SDL_SCANCODE_F2:        key.scan = SCAN_F2; break;
    case SDL_SCANCODE_F3:        key.scan = SCAN_F3; break;
    case SDL_SCANCODE_F4:        key.scan = SCAN_F4; break;
    case SDL_SCANCODE_F5:        key.scan = SCAN_F5; break;
    case SDL_SCANCODE_F6:        key.scan = SCAN_F6; break;
    case SDL_SCANCODE_F7:        key.scan = SCAN_F7; break;
    case SDL_SCANCODE_F8:        key.scan = SCAN_F8; break;
    case SDL_SCANCODE_F9:        key.scan = SCAN_F9; break;
    case SDL_SCANCODE_F10:       key.scan = SCAN_F10; break;
    case SDL_SCANCODE_F11:       key.scan = SCAN_F11; break;
    case SDL_SCANCODE_F12:       key.scan = SCAN_F12; break;
    case SDL_SCANCODE_ESCAPE:    key.scan = SCAN_ESC; break;
    // Other SDL scancodes have no UEFI mapping
    default: goto out;
    }

    // Add key to input queue
    self->input_queue[self->input_queue_count++] = key;

out:
    pthread_mutex_unlock(&self->input_queue_lock);
}

//
// Translate an SDL text input event to an efi_in_key
//
static void translate_unicode(ConsoleHandle *self, SDL_TextInputEvent *sdl_text)
{
    pthread_mutex_lock(&self->input_queue_lock);

    // Only translate event if it fits in the queue
    if (self->input_queue_count == ARRAY_SIZE(self->input_queue))
        goto out;

    // Add key to input queue
    efi_in_key *key = self->input_queue + self->input_queue_count++;

    // FIXME: this only translates ASCII chars
    key->scan = 0;
    key->c = sdl_text->text[0];

out:
    pthread_mutex_unlock(&self->input_queue_lock);
}

//
// This is the main function of the secondary thread we use to run the SDL
// event loop
//
static void *console_update(void *userdata)
{
    ConsoleHandle *self = userdata;
    while (self->keep_running) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
            switch (event.type) {
            case SDL_KEYDOWN:
                translate_scancode(self, &event.key);
                break;
            case SDL_TEXTINPUT:
                translate_unicode(self, &event.text);
                break;
            case SDL_QUIT:
                self->exit_callback(self);
                goto end;
            }

        // Render framebuffer contents
        SDL_UpdateTexture(self->texture,
            NULL, (void *) self->gop.mode->frame_buffer_base,
            sizeof(efi_u32) * self->gop.mode->info->pixels_per_scan_line);
        SDL_RenderClear(self->renderer);
        SDL_RenderCopy(self->renderer, self->texture, NULL, NULL);
        SDL_RenderPresent(self->renderer);
    }
end:
    return NULL;
}

//
// Create a new console instance
// WARNING: the exit callback is called from a different thread
//
ConsoleHandle *console_init(ConsoleExitCallback exit_callback)
{
    ConsoleHandle *self = malloc(sizeof *self);

    // Save exit callback for later
    self->exit_callback = exit_callback;

    // Setup SDL
    self->window = SDL_CreateWindow("UEFI Graphics Output",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GOPEMU_WIDTH, GOPEMU_HEIGHT, 0);
    self->renderer = SDL_CreateRenderer(self->window, -1, 0);
    self->texture = SDL_CreateTexture(self->renderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, GOPEMU_WIDTH, GOPEMU_HEIGHT);

    // Setup input queue
    pthread_mutex_init(&self->input_queue_lock, NULL);
    self->input_queue_count = 0;

    // Setup GOP
    self->gop.mode = &gopemu_mode;
    // Setup text input
    self->text_in.reset = text_in_reset;
    self->text_in.read_key = text_in_read_key;

    // Create event loop thread
    self->keep_running = 1;
    pthread_create(&self->loop_thread, NULL, console_update, self);

    return self;
}

efi_graphics_output_protocol *console_gop(ConsoleHandle *self)
{
    return &self->gop;
}

efi_simple_text_in_protocol *console_text_in(ConsoleHandle *self)
{
    return &self->text_in;
}


efi_simple_text_out_protocol *console_text_out(ConsoleHandle *self)
{
    return &self->text_out;
}

//
// Stop the console instance and exit
//
void console_exit(ConsoleHandle *self)
{
    // Wait for event loop to exit
    self->keep_running = 0;
    pthread_join(self->loop_thread, NULL);

    // Destroy input queue mutex
    pthread_mutex_destroy(&self->input_queue_lock);

    // Destroy window
    SDL_DestroyTexture(self->texture);
    SDL_DestroyRenderer(self->renderer);
    SDL_DestroyWindow(self->window);

    free(self);
}
