//
// Prints information about every key pressed by the user
//

#include <efi.h>
#include <efiutil.h>

efi_status_t efiapi efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
  efi_init(image_handle, system_table);

  efi_st->con_in->reset(efi_st->con_in, false);
  efi_st->con_out->clear_screen(efi_st->con_out);

  for (;;) {
    efi_in_key_t key;
    while (efi_st->con_in->read_key(efi_st->con_in, &key) == EFI_NOT_READY)
      ;

    if (key.scan == EFI_SCAN_NULL) {
      efi_print(L"Got unicode char: %d\n", key.c);
    } else {
      efi_print(L"Got scancode: %d\n", key.scan);
    }
  }

  return EFI_SUCCESS;
}
