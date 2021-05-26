//
// Interposer layer for EFI services
//

#ifndef EFIEMU_H
#define EFIEMU_H

//
// Entry point of an EFI image
//
typedef efi_status (efiapi *efi_image_entry)(efi_handle, efi_system_table *);

//
// Protocol services
//

efi_status efiapi uemu_install_protocol_interface(efi_handle *handle,
    efi_guid *protocol, efi_interface_type interface_type, void *interface);

efi_status efiapi uemu_handle_protocol(efi_handle handle, efi_guid *protocol,
    void **interface);

efi_status efiapi uemu_locate_handle(efi_locate_search_type search_type,
    efi_guid *protocol, void *search_key, efi_size *buffer_size,
    efi_handle *buffer);

efi_status efiapi uemu_locate_protocol(efi_guid *protocol, void *registration,
    void **interface);

efi_status efiapi uemu_reinstall_protocol_interface(efi_handle handle,
    efi_guid *protocol, void *old_interface, void *new_interface);

efi_status efiapi uemu_uninstall_protocol_interface(efi_handle handle,
    efi_guid *protocol, void *interface);

efi_status efiapi uemu_open_protocol(efi_handle handle, efi_guid *protocol,
    void **interface, efi_handle agent_handle, efi_handle controller_handle,
    efi_u32 attrib);

efi_status efiapi uemu_close_protocol(efi_handle handle, efi_guid *protocol,
    efi_handle agent_handle, efi_handle controller_handle);

efi_status efiapi uemu_open_protocol_information(efi_handle handle,
    efi_guid *protocol, efi_open_protocol_information_entry **entry_buffer,
    efi_size *entry_count);

//
// Variable services
//

efi_status efiapi uemu_get_next_variable_name(efi_size *variable_name_size,
    efi_ch16 *variable_name, efi_guid *vendor_guid);

//
// System table implementation provided by the EFI emulator
//
extern efi_system_table uemu_st;

#endif
