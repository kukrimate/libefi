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
#include "protocol/unicode.h"
#include "protocol/hii.h"
#include "peloader.h"

// Wait for key before exiting
static _Bool flag_wait = 0;

// Early exit mechasnism
static jmp_buf uemu_exit;

static void sigint_handler(int sig)
{
    longjmp(uemu_exit, 1);
}

static void sigtrap_handler(int sig)
{
    printf("Hanging in SIGTRAP!\n");
    for (;;)
        ;
}

static void console_exit_callback(ConsoleHandle *console_handle)
{
    kill(getpid(), SIGINT);
}

#define EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID \
    {0x379be4e,0xd706,0x437d,\
    {0xb0,0x37,0xed,0xb8,0x2f,0xb7,0x72,0xa4 }}

#define EFI_HII_STRING_PROTOCOL_GUID \
    { 0xfd96974, 0x23aa, 0x4cdc,\
    { 0xb9, 0xcb, 0x98, 0xd1, 0x77, 0x50, 0x32, 0x2a }}

#define EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID \
    { 0x587e72d7, 0xcc50, 0x4f79,\
    { 0x82, 0x09, 0xca, 0x29, 0x1f, 0xc1, 0xa1, 0x0f }}

static void start_emulator(void *image)
{
    signal(SIGINT, sigint_handler);
    signal(SIGTRAP, sigtrap_handler);

    // Initialize graphics output emulator
    ConsoleHandle *console_handle = console_init(console_exit_callback);

    efi_handle handle;

    // Install console protocols into emulator
    handle = NULL;
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
        efi_native_interface,
        console_gop(console_handle));

    uemu_st.con_in_handle = handle;
    uemu_st.con_in = console_text_in(console_handle);
    uemu_st.std_err_handle = handle;
    uemu_st.std_err = uemu_st.con_out = console_text_out(console_handle);

    // Install Unicode Collation protocol
    handle = NULL;
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_UNICODE_COLLATION_PROTOCOL2_GUID,
        efi_native_interface,
        unicode_collation());

    // Install HII protocols
    handle = NULL;
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_HII_DATABASE_PROTOCOL_GUID,
        efi_native_interface,
        hii_database());

    // Fake protocols for testing
    handle = NULL;
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID,
        efi_native_interface,
        (void *) 0xdeadbeef);
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_HII_STRING_PROTOCOL_GUID,
        efi_native_interface,
        (void *) 0xdeadbeef);
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_HII_CONFIG_ROUTING_PROTOCOL_GUID,
        efi_native_interface,
        (void *) 0xdeadbeef);

    // Create handle for the image itself
    handle = NULL;
    efi_loaded_image_protocol loaded_image = {0};
    uemu_install_protocol_interface(&handle,
        &(efi_guid) EFI_LOADED_IMAGE_PROTOCOL_GUID,
        efi_native_interface,
        &loaded_image);

    printf("%p\n", handle);

    // Setup early exit point
    if (setjmp(uemu_exit)) {
        printf("Exiting early...\n");
        goto setjmp_exit;
    }

    // Call image entry point
    efi_image_entry entry_point = get_pe_entry(image);
    efi_ssize exit_code = entry_point(handle, &uemu_st);
    if (EFI_ERROR(exit_code)) {
        // Pretend EFI error codes are just negative numbers
        exit_code &= ~SIZE_MAX_BIT;
        exit_code *= -1;
    }
    printf("Image exited with code: %ld\n", exit_code);

    // Wait for key if requested
    if (flag_wait) {
        printf("Press return to exit!\n");
        getchar();
    }

setjmp_exit:
    console_exit(console_handle);
}

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "hw")) != -1)
        switch (opt) {
        case 'w':
            flag_wait = 1;
            break;
        case 'h':
        default:
            goto print_usage;
        }

    if (optind >= argc) {
    print_usage:
        fprintf(stderr, "Usage: %s [-h] [-w] PEFILE\n", argv[0]);
        return 1;
    }

    // Open file
    int fd = open(argv[optind], O_RDONLY);
    if (fd < 0) {
        perror(argv[optind]);
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
