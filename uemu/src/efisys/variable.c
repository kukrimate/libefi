//
// Variable services
//

#include <efi.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

//
// EFI variable
//
typedef struct uemu_variable uemu_variable;
struct uemu_variable {
    efi_u32 attrib;
    efi_ch16 *name;
    efi_guid vendor_guid;
    efi_size data_size;
    void *data;
    uemu_variable *next;
};

// static uemu_variable lang = {
//     .name = L"PlatformLang",
//     .vendor_guid = EFI_GLOBAL_VARIABLE,
//     .next = NULL,
// };

// static uemu_variable lang_codes = {
//     .name = L"PlatformLangCodes",
//     .vendor_guid = EFI_GLOBAL_VARIABLE,
//     .next = &lang,
// };

static uemu_variable *variables = NULL;

efi_status efiapi uemu_get_variable(efi_ch16 *variable_name,
    efi_guid *vendor_guid, efi_u32 *attrib, efi_size *data_size, void *data)
{
    printf("rt->get_variable(%s, %s)\n",
        efi_to_ascii(variable_name), guid_to_ascii(vendor_guid));

    if (variable_name == NULL || vendor_guid == NULL || data_size == NULL)
        return EFI_INVALID_PARAMETER;

    // NOTE: the spec is not clear whether we should check for this condition
    // right away or only after we discover the actual size of the variable
    // is greater than zero
    if (*data_size > 0 && data == NULL)
        return EFI_INVALID_PARAMETER;

    // Search for matching variable
    uemu_variable *uvar = variables;
    while (uvar) {
        if (memcmp(&uvar->vendor_guid, vendor_guid, sizeof *vendor_guid) == 0
                    && efi_strcmp(uvar->name, variable_name) == 0)
        uvar = uvar->next;
    }
    if (!uvar)
        return EFI_NOT_FOUND;

    // Always copy attributes, even if the buffer is too small
    if (attrib)
        *attrib = uvar->attrib;

    // Make sure the buffer is large enough
    efi_size buffer_size = *data_size;
    *data_size = uvar->data_size;
    if (buffer_size < *data_size)
        return EFI_BUFFER_TOO_SMALL;

    memcpy(data, uvar->data, *data_size);
    return EFI_SUCCESS;
}

efi_status efiapi uemu_get_next_variable_name(efi_size *variable_name_size,
    efi_ch16 *variable_name, efi_guid *vendor_guid)
{
    if (!variable_name_size || !variable_name || !vendor_guid)
        return EFI_INVALID_PARAMETER;
    if (*variable_name_size < sizeof(efi_ch16))
        return EFI_INVALID_PARAMETER;
    // Verify that the string actually has a NUL terminator somewhere
    for (efi_size idx = 0; idx < *variable_name_size; ++idx)
        if (variable_name[idx] == 0)
            goto has_nul;
    return EFI_INVALID_PARAMETER;
has_nul:;

    uemu_variable *var = variables;
    // Unless we got an empty string, we must continue a previous search
    if (*variable_name != 0) {
        while (var) {
            if (memcmp(&var->vendor_guid, vendor_guid, sizeof *vendor_guid) == 0
                    && efi_strcmp(var->name, variable_name) == 0)
                goto found;
            var = var->next;
        }
        // The input values must name an existing variable
        return EFI_INVALID_PARAMETER;
found:
        // Continue searching at the next variable
        var = var->next;
    }

    // No more variables left
    if (!var)
        return EFI_NOT_FOUND;

    // Make sure the variable name fits into the buffer
    efi_size oldsize = *variable_name_size;
    *variable_name_size = efi_strsize(var->name);
    if (oldsize < *variable_name_size)
        return EFI_BUFFER_TOO_SMALL;

    // Copy variable name and GUID
    memcpy(variable_name, var->name, *variable_name_size);
    memcpy(vendor_guid, &var->vendor_guid, sizeof *vendor_guid);
    return EFI_SUCCESS;
}
