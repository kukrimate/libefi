#include <efi.h>
#include <efiutil.h>

efi_status_t efiapi efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
  efi_init(image_handle, system_table);

  efi_status_t status = EFI_SUCCESS;
  efi_size_t varcnt = 0;

  // Empty string starts a search
  efi_size_t old_name_size;
  efi_size_t var_name_size = sizeof(efi_ch16_t);
  efi_ch16_t *var_name = efi_alloc(var_name_size);
  var_name[0] = 0;

  // Returned GUID
  efi_guid_t vendor_guid;

  for (;;) {
    // Get name and GUID for this variable
retry:
    old_name_size = var_name_size;
    status = efi_rt->get_next_variable_name(&var_name_size, var_name, &vendor_guid);
    if (status == EFI_BUFFER_TOO_SMALL) { // We need a bigger buffer
      var_name = efi_realloc(var_name, old_name_size, var_name_size);
      goto retry;
    } else if (status == EFI_NOT_FOUND) { // End of variables
      efi_free(var_name);
      break;
    } else if (EFI_ERROR(status)) { // Some other error
      return status;
    }
    efi_print(L"%g %s\n", vendor_guid, var_name);
    ++varcnt;
  }

  efi_print(L"# of variables printed: %ld\n", varcnt);
  return EFI_SUCCESS;
}
