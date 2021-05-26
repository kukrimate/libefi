//
// Implementation of the UEFI Human Interface Infrastructure
//

#include <efi.h>
#include <stdio.h>
#include <stdlib.h>
#include "hii.h"

static efi_status efiapi new_package_list(efi_hii_database_protocol *self,
    efi_hii_package_list_header *package_list, efi_handle driver_handle,
    efi_hii_handle *handle)
{
    printf("hii_db->new_package_list()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi remove_package_list(efi_hii_database_protocol *self,
    efi_hii_handle handle)
{
    printf("hii_db->remove_package_list()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi update_package_list(efi_hii_database_protocol *self,
    efi_hii_handle handle,
    efi_hii_package_list_header *package_list)
{
    printf("hii_db->update_package_list()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi list_package_lists(efi_hii_database_protocol *self,
    efi_u8 package_type, efi_guid *package_guid,
    efi_size *buffer_size, efi_hii_handle *buffer)
{
    printf("hii_db->list_package_lists()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi export_package_lists(efi_hii_database_protocol *self,
    efi_hii_handle handle, efi_size *buffer_size,
    efi_hii_package_list_header *buffer)
{
    printf("hii_db->export_package_lists()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi register_package_notify(efi_hii_database_protocol *self,
    efi_u8 package_type, efi_guid *package_guid,
    efi_hii_database_notify notify_function,
    efi_size notify_type, efi_handle *notify_handle)
{
    printf("hii_db->register_package_notify()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi unregister_package_notify(efi_hii_database_protocol *self,
    efi_handle notify_handle)
{
    printf("hii_db->unregister_package_notify()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi find_keyboard_layouts(efi_hii_database_protocol *self,
    efi_u16 *key_guid_buffer_length,
    efi_guid *key_guid_buffer)
{
    printf("hii_db->find_keyboard_layouts()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi get_keyboard_layout(efi_hii_database_protocol *self,
    efi_guid *key_guid, efi_u16 *keyboard_layout_length,
    void *keyboard_layout)
{
    printf("hii_db->get_keyboard_layout()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi set_keyboard_layout(efi_hii_database_protocol *self,
    efi_guid *key_guid)
{
    printf("hii_db->set_keyboard_layout()\n");
    return EFI_UNSUPPORTED;
}

static efi_status efiapi get_package_list_handle(efi_hii_database_protocol *self,
    efi_hii_handle package_list_handle, efi_handle *driver_handle)
{
    printf("hii_db->get_package_list_handle()\n");
    return EFI_UNSUPPORTED;
}

static efi_hii_database_protocol uemu_hii_database = {
    .new_package_list = new_package_list,
    .remove_package_list = remove_package_list,
    .update_package_list = update_package_list,
    .list_package_lists = list_package_lists,
    .export_package_lists = export_package_lists,
    .register_package_notify = register_package_notify,
    .unregister_package_notify = unregister_package_notify,
    .find_keyboard_layouts = find_keyboard_layouts,
    .get_keyboard_layout = get_keyboard_layout,
    .set_keyboard_layout = set_keyboard_layout,
    .get_package_list_handle = get_package_list_handle,
};

efi_hii_database_protocol *hii_database(void)
{
    return &uemu_hii_database;
}
