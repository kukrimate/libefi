/*
 * User mode UEFI emulator
 * Author: Mate Kukri
 */

#ifndef __amd64__
#error The user mode emulator only supports AMD64
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <efi.h>
#include "efiemu.h"
#include "protocol/console.h"
#include "peloader.h"

// Early exit mechasnism
static jmp_buf uemu_exit;

static void sigint_handler(int sig)
{
    longjmp(uemu_exit, 1);
}

static void console_exit_callback(ConsoleHandle *console_handle)
{
    kill(getpid(), SIGINT);
}

static void start_emulator(void *image)
{
    // Catch SIGINT
    signal(SIGINT, sigint_handler);

    // Initialize graphics output emulator
    ConsoleHandle *console_handle = console_init(console_exit_callback);

    // Install console protocols into emulator
    uemu_expose_protocol(
        &(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
        console_gop(console_handle));
    uemu_st.con_in = console_text_in(console_handle);

    // Setup early exit point
    if (setjmp(uemu_exit)) {
        printf("Exiting early...\n");
        goto setjmp_exit;
    }

    // Call image entry point
    efi_image_entry entry_point = get_pe_entry(image);
    efi_ssize exit_code = entry_point(NULL, &uemu_st);
    if (EFI_ERROR(exit_code)) {
        // Pretend EFI error codes are just negative numbers
        exit_code &= ~SIZE_MAX_BIT;
        exit_code *= -1;
    }
    printf("Image exited with code: %ld\n", exit_code);

setjmp_exit:
    console_exit(console_handle);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s PEFILE\n", argv[0]);
        return 1;
    }

    // Open file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror(argv[1]);
        return 1;
    }

    // Load image
    void *image = load_pe_image(fd);
    if (!image) {
        close(fd);
        return 1;
    }

    // Close file now that the image is loaded
    close(fd);

    // Start emulator on newly loaded image
    start_emulator(image);

    return 0;
}
