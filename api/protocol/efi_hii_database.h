//
// Human Interface Infrastructure database protocol
//

#ifndef EFI_HII_DATABASE_H
#define EFI_HII_DATABASE_H

#define EFI_HII_DATABASE_PROTOCOL_GUID \
    { 0xef9fc172, 0xa1b2, 0x4693, 0xb3, 0x27, 0x6d, 0x32, 0xfc, 0x41, \
        0x60, 0x42 }

typedef struct {
    void *new_package_list;
    void *remove_package_list;
    void *update_package_list;
    void *list_package_lists;
    void *export_package_lists;
    void *register_package_notify;
    void *unregister_package_notify;
    void *find_keyboard_layouts;
    void *get_keyboard_layout;
    void *set_keyboard_layout;
    void *get_package_list_handle;
} efi_hii_database_protocol;

#endif
