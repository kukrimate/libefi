/*
 * Interposer layer for (U)EFI services
 */

#include <stdio.h>
#include <efi.h>

static efi_status efiapi unsupported_stub()
{
    return EFI_UNSUPPORTED;
}

efi_boot_services uemu_bs = {
    .wait_for_event = unsupported_stub,
};

static efi_status efiapi uemu_clear_screen(efi_simple_text_out_protocol *self)
{
    return EFI_SUCCESS;
}

static efi_status efiapi uemu_output_string(efi_simple_text_out_protocol *self,
                                            efi_ch16 *str)
{
    for (; *str; ++str)
        putchar(*str);

    return EFI_SUCCESS;
}

efi_simple_text_out_protocol uemu_con_out = {
    .clear_screen = &uemu_clear_screen,
    .output_string = &uemu_output_string,
};

efi_system_table uemu_st = {
    .con_out = &uemu_con_out,
    .boot_services = &uemu_bs,
};
