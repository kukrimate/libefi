/*
 * Hello World for UEFI
 */

#include <efi.h>
#include <efiutil.h>

efi_status_t efiapi efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
  efi_size_t index;
  efi_in_key_t key;

  efi_init(image_handle, system_table);
  st->con_out->clear_screen(st->con_out);
  efi_print(L"Hello, World from UEFI!\nPress any key to exit\n");
  bs->wait_for_event(1, &st->con_in->wait_for_key, &index);
  st->con_in->read_key(st->con_in, &key);
  return EFI_SUCCESS;
}
