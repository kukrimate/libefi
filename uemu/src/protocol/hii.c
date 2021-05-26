//
// Implementation of the UEFI Human Interface Infrastructure
//

#include <efi.h>
#include <stdio.h>
#include <stdlib.h>
#include "hii.h"

static efi_status efiapi hii_stub()
{
    // printf("HII database call!\n");
    return EFI_UNSUPPORTED;
}

static efi_hii_database_protocol uemu_hii_database = {
    .new_package_list = hii_stub,
    .remove_package_list = hii_stub,
    .update_package_list = hii_stub,
    .list_package_lists = hii_stub,
    .export_package_lists = hii_stub,
    .register_package_notify = hii_stub,
    .unregister_package_notify = hii_stub,
    .find_keyboard_layouts = hii_stub,
    .get_keyboard_layout = hii_stub,
    .set_keyboard_layout = hii_stub,
    .get_package_list_handle = hii_stub,
};

efi_hii_database_protocol *hii_database(void)
{
    return &uemu_hii_database;
}


// 0x17A56B07 - 0x17a02000 = 0x54b07
// 0x17A5D953 - 0x17a02000 = 0x5b953
