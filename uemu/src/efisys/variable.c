//
// Variable services
//

#include <efi.h>
#include <string.h>
#include "util.h"

//
// EFI variable
//
typedef struct uemu_variable uemu_variable;
struct uemu_variable {
    efi_u32  attrib;
    efi_ch16 *name;
    efi_guid vendor_guid;
    uemu_variable *next;
};

static uemu_variable test_var3 = {
    .name = L"VariableNo3",
    .vendor_guid = { 0xcafebabe, 0xcafe, 0xdead, 0xff, 0xee,
                        0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88 },
    .next = NULL,
};

static uemu_variable test_var2 = {
    .name = L"TestVariable2",
    .vendor_guid = { 0xdeadbeef, 0xcafe, 0xdead, 0xff, 0xee,
                        0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88 },
    .next = &test_var3,
};

static uemu_variable test_var1 = {
    .name = L"TestVariable1",
    .vendor_guid = { 0xdeadbeef, 0xcafe, 0xdead, 0xff, 0xee,
                        0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88 },
    .next = &test_var2,
};

static uemu_variable *variables = &test_var1;

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
