//
// Utility functions for EFI data structures
//

#include <stdio.h>
#include <efi.h>
#include "util.h"

//
// Compare two EFI strings
//
efi_ssize efi_strcmp(efi_ch16 *str1, efi_ch16 *str2)
{
    while (*str1 == *str2++) {
        if (*str1++ == 0) {
            return 0;
        }
    }
    return *str1 - str2[-1];
}

//
// Compute the number of characters (excluding the NUL-terminator) in an EFI string
//
efi_size efi_strlen(efi_ch16 *str)
{
    efi_ch16 *p = str;
    for (; *p; ++p);
    return p - str;
}

//
// Compute the size of an EFI string (including the NUL-terminator) in bytes
//
efi_size efi_strsize(efi_ch16 *str)
{
    return (efi_strlen(str) + 1) * sizeof(efi_ch16);
}

//
// Convert an EFI string to an ASCII string
//
char *efi_to_ascii(efi_ch16 *str)
{
    static char buf[4096];
    char *p = buf;

    while (*str)
        *p++ = *str++;
    *p = 0;
    return buf;
}

//
// Convert an EFI GUID to an ASCII string
//
char *guid_to_ascii(efi_guid *guid)
{
    static char buf[4096];

    snprintf(buf, sizeof buf,
                    "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                    guid->data1, guid->data2, guid->data3,
                    guid->data4[0], guid->data4[1],
                    guid->data4[2], guid->data4[3],
                    guid->data4[4], guid->data4[5],
                    guid->data4[6], guid->data4[7]);
    return buf;
}
