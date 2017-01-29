/*
 * EFI simple file system protocol
 */

#ifndef __EFI_SIMPLE_FILE_SYSTEM_H
#define __EFI_SIMPLE_FILE_SYSTEM_H

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
  { 0x0964e5b22, 0x6459, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, \
    0xc9, 0x69, 0x72, 0x3b }

typedef struct _efi_simple_file_system_protocol efi_simple_file_system_protocol_t;
struct _efi_simple_file_system_protocol {
	uint64_t revision;
	efi_status_t (efi_func *open_volume) (efi_simple_file_system_protocol_t *self, efi_file_protocol_t **root);
};

#endif
