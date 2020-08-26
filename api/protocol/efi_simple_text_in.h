/*
 * EFI simple text input protocol
 */
#ifndef EFI_SIMPLE_TEXT_IN_H
#define EFI_SIMPLE_TEXT_IN_H

typedef struct {
	efi_u16		scan;
	efi_ch16	c;
} efi_in_key;

typedef struct efi_simple_text_in_protocol efi_simple_text_in_protocol;
struct efi_simple_text_in_protocol {
	efi_status (efiapi *reset)    (efi_simple_text_in_protocol *self, efi_bool ext_verf);
	efi_status (efiapi *read_key) (efi_simple_text_in_protocol *self, efi_in_key *key);
	efi_event	wait_for_key;
};

#endif
