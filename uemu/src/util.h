//
// Utility functions for EFI data structures
//

#ifndef UTIL_H
#define UTIL_H

//
// Calculate the size of an array
//
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

//
// Compare two EFI strings
//
efi_ssize efi_strcmp(efi_ch16 *str1, efi_ch16 *str2);

//
// Compute the number of characters (excluding the NUL-terminator) in an EFI string
//
efi_size efi_strlen(efi_ch16 *str);

//
// Compute the size of an EFI string (including the NUL-terminator) in bytes
//
efi_size efi_strsize(efi_ch16 *str);

//
// Convert an EFI string to an ASCII string
//
char *efi_to_ascii(efi_ch16 *str);

//
// Convert an EFI GUID to an ASCII string
//
char *guid_to_ascii(efi_guid *guid);

#endif
