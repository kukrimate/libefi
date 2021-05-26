//
// Interposer layer for EFI services
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efi.h>
#include "efiemu.h"
#include "util.h"

//
// Not implemented yet
//

static efi_status efiapi unsupported_stub()
{
    return EFI_UNSUPPORTED;
}

//
// Memory allocation
//

static efi_status efiapi uemu_allocate_pool(efi_memory_type pool_type,
    efi_size size, void **buffer)
{
    printf("bs->allocate_pool(%d, %ld, %p)\n", pool_type, size, buffer);

    if (pool_type < efi_reserved_memory_type
            || pool_type >= efi_max_memory_type
            || buffer == NULL)
        return EFI_INVALID_PARAMETER;

    *buffer = malloc(size);
    if (*buffer == NULL)
        return EFI_OUT_OF_RESOURCES;

    return EFI_SUCCESS;
}

static efi_status efiapi uemu_free_pool(void *buffer)
{
    printf("bs->free_pool(%p)\n", buffer);

    free(buffer);
    return EFI_SUCCESS;
}

static efi_status efiapi uemu_wait_for_event(efi_size num_events,
    efi_event *event, efi_size *index)
{
    return EFI_INVALID_PARAMETER;
}


static efi_status efiapi uemu_calculate_crc32(void *data, efi_size data_size,
    efi_u32 *crc32)
{
    printf("WARN: bs->calculate_crc32() unsupported!\n");
    return EFI_UNSUPPORTED;
}

static void efiapi uemu_copy_mem(void *dest, void *src, efi_size length)
{
    printf("bs->copy_mem(%p, %p, %ld)\n", dest, src, length);
    memcpy(dest, src, length);
}

static void efiapi uemu_set_mem(void *buffer, efi_size size, efi_u8 value)
{
    printf("bs->set_mem(%p, %ld, %02x)\n", buffer, size, value);
    memset(buffer, value, size);
}

efi_boot_services uemu_bs = {
    // TPL services
    .raise_tpl   = unsupported_stub,
    .restore_tpl = unsupported_stub,

    // Memory allocation services
    .allocate_pages = unsupported_stub,
    .free_pages     = unsupported_stub,
    .get_memory_map = unsupported_stub,
    .allocate_pool  = uemu_allocate_pool,
    .free_pool      = uemu_free_pool,

    // Event services
    .create_event   = unsupported_stub,
    .set_timer      = unsupported_stub,
    .wait_for_event = uemu_wait_for_event,
    .signal_event   = unsupported_stub,
    .close_event    = unsupported_stub,
    .check_event    = unsupported_stub,

    // Protocol services
    .install_protocol_interface   = uemu_install_protocol_interface,
    .reinstall_protocol_interface = uemu_reinstall_protocol_interface,
    .uninstall_protocol_interface = uemu_uninstall_protocol_interface,
    .handle_protocol              = uemu_handle_protocol,
    .reserved                     = unsupported_stub,
    .register_protocol_notify     = unsupported_stub,
    .locate_handle                = uemu_locate_handle,
    .locate_device_path           = unsupported_stub,
    .install_configuration_table  = unsupported_stub,

    // Image services
    .load_image         = unsupported_stub,
    .start_image        = unsupported_stub,
    .exit               = unsupported_stub,
    .unload_image       = unsupported_stub,
    .exit_boot_services = unsupported_stub,

    // Misc services
    .get_next_monotonic_count = unsupported_stub,
    .stall                    = unsupported_stub,
    .set_watchdog_timer       = unsupported_stub,

    // Driver support services
    .connect_controller    = unsupported_stub,
    .disconnect_controller = unsupported_stub,

    // Open and close protocol services
    .open_protocol             = uemu_open_protocol,
    .close_protocol            = uemu_close_protocol,
    .open_protocol_information = uemu_open_protocol_information,

    // Library services
    .protocols_per_handle                   = unsupported_stub,
    .locate_handle_buffer                   = unsupported_stub,
    .locate_protocol                        = uemu_locate_protocol,
    .install_multiple_protocol_interfaces   = unsupported_stub,
    .uninstall_multiple_protocol_interfaces = unsupported_stub,

    // CRC32
    .calculate_crc32 = uemu_calculate_crc32,

    // Misc services
    .copy_mem =        uemu_copy_mem,
    .set_mem =         uemu_set_mem,

    // UEFI 2.0+ event service
    .create_event_ex = unsupported_stub,
};

efi_runtime_services uemu_rt = {
    .get_time = unsupported_stub,
    .set_time = unsupported_stub,
    .get_wakeup_time = unsupported_stub,
    .set_wakeup_time = unsupported_stub,

    .set_virtual_address_map = unsupported_stub,
    .convert_pointer = unsupported_stub,

    .get_variable = unsupported_stub,
    .get_next_variable_name = uemu_get_next_variable_name,
    .set_variable = unsupported_stub,

    .get_next_high_monotonic_count = unsupported_stub,
    .reset_system = unsupported_stub,

    .update_capsule = unsupported_stub,
    .query_capsule_capabilities = unsupported_stub,

    .query_variable_info = unsupported_stub,
};

efi_system_table uemu_st = {
    .vendor = L"libefi user-mode emulator",
    .boot_services = &uemu_bs,
    .runtime_services = &uemu_rt,
};
