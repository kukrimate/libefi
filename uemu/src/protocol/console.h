//
// Emulator for the UEFI console using SDL2, it implements the following protocols:
//  * EFI Graphics Output Protocol (GOP)
//  * EFI Simple Text Input Protocol
//  * EFI Simple Text Output Protocol
//

#ifndef CONSOLE_H
#define CONSOLE_H

//
// Opaque handle for the console
//
typedef struct ConsoleHandle ConsoleHandle;

//
// Callback to call when a console user requests a window close
//
typedef void (*ConsoleExitCallback)(ConsoleHandle *self);

//
// Create a new console instance
// WARNING: the exit callback is called from a different thread
//
ConsoleHandle *console_init(ConsoleExitCallback exit_callback);

//
// Get pointers to protocols provided by the console emulator
//
efi_graphics_output_protocol *console_gop(ConsoleHandle *self);
efi_simple_text_in_protocol *console_text_in(ConsoleHandle *self);
efi_simple_text_out_protocol *console_text_out(ConsoleHandle *self);

//
// Stop the console instance and exit
//
void console_exit(ConsoleHandle *self);

#endif
