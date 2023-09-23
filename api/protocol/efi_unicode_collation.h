//
// Unicode Collation Protocol
//

#ifndef EFI_UNICODE_COLLATION_H
#define EFI_UNICODE_COLLATION_H

#define EFI_UNICODE_COLLATION_PROTOCOL2_GUID \
  { 0xa4c751fc, 0x23ae, 0x4c3e, 0x92, 0xe9, 0x49, 0x64, 0xcf, 0x63, 0xf3, 0x49 }

typedef struct efi_unicode_collation_protocol efi_unicode_collation_protocol_t;

struct efi_unicode_collation_protocol {
  // Compare two Unicode strings in a case-insensitive manner
  efi_ssize_t (efiapi *stri_coll)(efi_unicode_collation_protocol_t *self,
    efi_ch16_t *s1, efi_ch16_t *s2);

  // Match a unicode string against a pattern
  efi_bool_t (efiapi *metai_match)(efi_unicode_collation_protocol_t *self,
    efi_ch16_t *string, efi_ch16_t *pattern);

  // Convert a Unicode string in-place to lower-case
  void (efiapi *str_lwr)(efi_unicode_collation_protocol_t *self,
    efi_ch16_t *string);

  // Convert a Unicode string in-place to upper-case
  void (efiapi *str_upr)(efi_unicode_collation_protocol_t *self,
    efi_ch16_t *string);

  // Convert a FAT filename to a Unicode string
  void (efiapi *fat_to_str)(efi_unicode_collation_protocol_t *self,
    efi_size_t fat_size, efi_ch8_t *fat, efi_ch16_t *string);

  // Convert a Unicode string to a FAT filename
  efi_bool_t (efiapi *str_to_fat)(efi_unicode_collation_protocol_t *self,
    efi_ch16_t *string, efi_size_t fat_size, efi_ch8_t *fat);

  // Language codes supported by this instance
  efi_ch8_t *supported_languages;
};

#endif
