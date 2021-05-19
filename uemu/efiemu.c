/*
 * Interposer layer for (U)EFI services
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efi.h>

//
// Services that were not implemented yet
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
    free(buffer);
    return EFI_SUCCESS;
}

static efi_status efiapi uemu_wait_for_event(efi_size num_events,
                                             efi_event *event,
                                             efi_size *index)
{
    return EFI_INVALID_PARAMETER;
}

//
// Protocol services
//

typedef struct uemu_protocol uemu_protocol;
struct uemu_protocol {
    efi_guid *guid;
    void     *interface;
    uemu_protocol *next;
};

typedef struct uemu_handle uemu_handle;
struct uemu_handle {
    uemu_protocol *protocols;
    uemu_handle *next;
};

// Start with an empty handle database
static uemu_handle *handles = NULL;

static uemu_handle *uemu_new_handle(void)
{
    uemu_handle *new_handle = malloc(sizeof *new_handle);
    new_handle->next = handles;
    handles = new_handle;
    return new_handle;
}

void uemu_expose_protocol(efi_guid *with_guid, void *interface)
{
    uemu_handle *handle = uemu_new_handle();
    uemu_protocol *proto = malloc(sizeof *proto);
    proto->guid = with_guid;
    proto->interface = interface;
    proto->next = NULL;
    handle->protocols = proto;
}

static efi_status efiapi uemu_handle_protocol(efi_handle handle,
                                              efi_guid *protocol,
                                              void **interface)
{
    // Invalid handle, this error code is in the spec
    if (handle == NULL)
        return EFI_INVALID_PARAMETER;

    // Search for protocol
    uemu_protocol *proto = ((uemu_handle *) handle)->protocols;
    while (proto) {
        if (!memcmp(proto->guid, protocol, sizeof *protocol)) {
            *interface = proto->interface;
            return EFI_SUCCESS;
        }
        proto = proto->next;
    }

    // No protocol with the requested GUID, error code not specified but EDK2
    // also returns this
    return EFI_UNSUPPORTED;
}

static _Bool handle_has_guid(uemu_handle *handle, efi_guid *guid)
{
    uemu_protocol *proto = handle->protocols;
    while (proto) {
        if (!memcmp(proto->guid, guid, sizeof *guid))
            return 1;
        proto = proto->next;
    }
    return 0;
}

static efi_status efiapi uemu_locate_handle(efi_locate_search_type search_type,
                                            efi_guid *protocol,
                                            void *search_key,
                                            efi_size *buffer_size,
                                            efi_handle *buffer)
{
    if (search_type == by_register_notify)
        return EFI_UNSUPPORTED;

    // First we count the handles
    efi_size handle_cnt = 0;
    for (uemu_handle *cur = handles; cur; cur = cur->next) {
        if (search_type == all_handles || handle_has_guid(cur, protocol)) {
            ++handle_cnt;
        }
    }

    // Calculate necessary buffer size
    efi_size orig_buffer_size = *buffer_size;
    *buffer_size = handle_cnt * sizeof(efi_handle);

    // Return indicator if we can't fit all handles
    if (orig_buffer_size < *buffer_size) {
        return EFI_BUFFER_TOO_SMALL;
    }

    // Return handles
    efi_size handle_idx = 0;
    for (uemu_handle *cur = handles; cur; cur = cur->next) {
        if (search_type == all_handles || handle_has_guid(cur, protocol)) {
            buffer[handle_idx] = (efi_handle) cur;
        }
    }
    return EFI_SUCCESS;
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
    .install_protocol_interface   = unsupported_stub,
    .reinstall_protocol_interface = unsupported_stub,
    .uninstall_protocol_interface = unsupported_stub,
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
};

static efi_status efiapi uemu_clear_screen(efi_simple_text_out_protocol *self)
{
    return EFI_SUCCESS;
}

static efi_status efiapi uemu_output_string(efi_simple_text_out_protocol *self,
                                            efi_ch16 *str)
{
    for (; *str; ++str)
        putchar(*str);

    return EFI_SUCCESS;
}

efi_simple_text_out_protocol uemu_con_out = {
    .clear_screen = &uemu_clear_screen,
    .output_string = &uemu_output_string,
};

efi_system_table uemu_st = {
    .con_out = &uemu_con_out,
    .boot_services = &uemu_bs,
};
