//
// Interposer layer for EFI services
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efi.h>
#include "util.h"

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
    for (;;)
        ;
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
    // printf("bs->locate_handle(%d, %s)\n", search_type, guid_to_ascii(protocol));

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

//
// Variable services
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

static efi_status efiapi uemu_get_next_variable_name(
        efi_size *variable_name_size,
        efi_ch16 *variable_name,
        efi_guid *vendor_guid)
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
};

efi_system_table uemu_st = {
    .vendor = L"libefi user-mode emulator",
    .boot_services = &uemu_bs,
    .runtime_services = &uemu_rt,
};
