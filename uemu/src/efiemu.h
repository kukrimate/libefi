//
// Interposer layer for EFI services
//

#ifndef EFIEMU_H
#define EFIEMU_H

//
// Entry point of an EFI image
//
typedef efi_status (efiapi *efi_image_entry)(efi_handle, efi_system_table *);

//
// System table implementation provided by the EFI emulator
//
extern efi_system_table uemu_st;

//
// Install a protocol into the EFI emulator
//
void uemu_expose_protocol(efi_guid *with_guid, void *interface);

#endif
