/*
 * Hello World for UEFI
 */

#include <efi.h>
#include <efiutil.h>

efi_status efiapi efi_main(efi_handle image_handle,
                           efi_system_table *system_table)
{
    efi_size index;

    efi_init(image_handle, system_table);
    st->con_out->clear_screen(st->con_out);
    efi_print(L"Hello, World from UEFI!\nPress any key to exit\n");
    bs->wait_for_event(1, &st->con_in->wait_for_key, &index);
    return EFI_SUCCESS;
}
