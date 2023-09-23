//
// Human Interface Infrastructure Database Protocol
//

#ifndef EFI_HII_DATABASE_H
#define EFI_HII_DATABASE_H

#define EFI_HII_DATABASE_PROTOCOL_GUID \
  { 0xef9fc172, 0xa1b2, 0x4693, 0xb3, 0x27, 0x6d, 0x32, 0xfc, 0x41, 0x60, 0x42 }

typedef void *efi_hii_handle_t;

//
// HII Package
//

typedef struct {
  efi_u32_t length : 24;
  efi_u32_t type   : 8;
  efi_u8_t  data[1];
} efi_hii_package_header_t;

#define EFI_HII_PACKAGE_TYPE_ALL             0x00
#define EFI_HII_PACKAGE_TYPE_GUID            0x01
#define EFI_HII_PACKAGE_FORM                 0x02
#define EFI_HII_PACKAGE_KEYBOARD_LAYOUT      0x03
#define EFI_HII_PACKAGE_STRINGS              0x04
#define EFI_HII_PACKAGE_FONTS                0x05
#define EFI_HII_PACKAGE_IMAGES               0x06
#define EFI_HII_PACKAGE_SIMPLE_FONTS         0x07
#define EFI_HII_PACKAGE_DEVICE_PATH          0x08
#define EFI_HII_PACKAGE_END                  0xDF
#define EFI_HII_PACKAGE_TYPE_SYSTEM_BEGIN    0xE0
#define EFI_HII_PACKAGE_TYPE_SYSTEM_END      0xFF

//
// HII Package List
//
typedef struct {
  efi_guid_t package_list_guid;
  efi_u32_t  package_length;
} efi_hii_package_list_header_t;

//
// HII Database Notification
//

#define EFI_HII_DATABASE_NOTIFY_NEW_PACK    0x00000001
#define EFI_HII_DATABASE_NOTIFY_REMOVE_PACK 0x00000002
#define EFI_HII_DATABASE_NOTIFY_EXPORT_PACK 0x00000004
#define EFI_HII_DATABASE_NOTIFY_ADD_PACK    0x00000008

typedef efi_status_t (efiapi *efi_hii_database_notify_t)(
  efi_u8_t package_type, efi_guid_t *package_guid,
  efi_hii_package_header_t *package, efi_hii_handle_t handle,
  efi_size_t notify_type);

typedef struct efi_hii_database_protocol efi_hii_database_protocol_t;

struct efi_hii_database_protocol {
  efi_status_t (efiapi *new_package_list)(efi_hii_database_protocol_t *self,
    efi_hii_package_list_header_t *package_list, efi_handle_t driver_handle,
    efi_hii_handle_t *handle);
  efi_status_t (efiapi *remove_package_list)(efi_hii_database_protocol_t *self,
    efi_hii_handle_t handle);
  efi_status_t (efiapi *update_package_list)(efi_hii_database_protocol_t *self,
    efi_hii_handle_t handle, efi_hii_package_list_header_t *package_list);
  efi_status_t (efiapi *list_package_lists)(efi_hii_database_protocol_t *self,
    efi_u8_t package_type, efi_guid_t *package_guid, efi_size_t *buffer_size,
    efi_hii_handle_t *buffer);
  efi_status_t (efiapi *export_package_lists)(efi_hii_database_protocol_t *self,
    efi_hii_handle_t handle, efi_size_t *buffer_size,
    efi_hii_package_list_header_t *buffer);
  efi_status_t (efiapi *register_package_notify)(
    efi_hii_database_protocol_t *self, efi_u8_t package_type,
    efi_guid_t *package_guid, efi_hii_database_notify_t notify_function,
    efi_size_t notify_type, efi_handle_t *notify_handle);
  efi_status_t (efiapi *unregister_package_notify)(
    efi_hii_database_protocol_t *self, efi_handle_t notify_handle);
  efi_status_t (efiapi *find_keyboard_layouts)(
    efi_hii_database_protocol_t *self, efi_u16_t *key_guid_buffer_length,
    efi_guid_t *key_guid_buffer);
  efi_status_t (efiapi *get_keyboard_layout)(efi_hii_database_protocol_t *self,
    efi_guid_t *key_guid, efi_u16_t *keyboard_layout_length,
    void *keyboard_layout);
  efi_status_t (efiapi *set_keyboard_layout)(efi_hii_database_protocol_t *self,
    efi_guid_t *key_guid);
  efi_status_t (efiapi *get_package_list_handle)(
    efi_hii_database_protocol_t *self, efi_hii_handle_t package_list_handle,
    efi_handle_t *driver_handle);
};

#endif
