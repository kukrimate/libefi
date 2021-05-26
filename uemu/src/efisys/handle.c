//
// Handle and protocol services
//

#include <efi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

//
// Installed protocol instance
//
typedef struct uemu_protocol uemu_protocol;
struct uemu_protocol {
    efi_guid *guid;
    void     *interface;
    uemu_protocol *next;
};

//
// Handle in the handle database
//
typedef struct uemu_handle uemu_handle;
struct uemu_handle {
    uemu_protocol *protocols;
    uemu_handle *next;
};

//
// Head of the handle list
//
static uemu_handle *uhandle_head = NULL;

//
// Private helper functions
//

static uemu_handle *create_uhandle(void)
{
    uemu_handle *uhandle = malloc(sizeof *uhandle);
    uhandle->protocols = NULL;
    uhandle->next = uhandle_head;
    uhandle_head = uhandle;
    return uhandle;
}

static uemu_protocol *find_uprotocol(uemu_handle *uhandle, efi_guid *guid)
{
    uemu_protocol *uprotocol = uhandle->protocols;
    while (uprotocol) {
        if (memcmp(uprotocol->guid, guid, sizeof *guid) == 0)
            return uprotocol;
        uprotocol = uprotocol->next;
    }
    return NULL;
}

static _Bool remove_uprotocol(uemu_handle *uhandle, efi_guid *guid)
{
    uemu_protocol **uprotocol_ptr = &uhandle->protocols;
    while (*uprotocol_ptr) {
        if (memcmp((*uprotocol_ptr)->guid, guid, sizeof *guid) == 0) {
            *uprotocol_ptr = (*uprotocol_ptr)->next;
            return 1;
        }
        uprotocol_ptr = &(*uprotocol_ptr)->next;
    }
    return 0;
}

static void install_protocol(uemu_handle *uhandle, efi_guid *guid, void *interface)
{
    uemu_protocol *uprotocol = malloc(sizeof *uprotocol);
    uprotocol->guid = guid;
    uprotocol->interface = interface;
    uprotocol->next = uhandle->protocols;
    uhandle->protocols = uprotocol;
}

//
// Functions exposed via the system table
//

efi_status efiapi uemu_install_protocol_interface(efi_handle *handle,
    efi_guid *protocol, efi_interface_type interface_type, void *interface)
{
    printf("bs->install_protocol_interface(%p, %s, %p)\n", handle,
        guid_to_ascii(protocol), interface);

    if (handle == NULL || protocol == NULL || interface_type != efi_native_interface)
        return EFI_INVALID_PARAMETER;

    // Find or create handle
    uemu_handle *uhandle;
    if (*handle == NULL) { // Create new handle
        uhandle = create_uhandle();
        *handle = (efi_handle) uhandle;
    } else {               // Use existing handle
        uhandle = (uemu_handle *) *handle;
    }

    // Make sure the handle doesn't have a protocol already installed with
    // a matching guid
    if (find_uprotocol(uhandle, protocol))
        return EFI_INVALID_PARAMETER;

    // Install protocol on handle, than indicate success
    install_protocol(uhandle, protocol, interface);
    return EFI_SUCCESS;
}

efi_status efiapi uemu_reinstall_protocol_interface(efi_handle handle,
    efi_guid *protocol, void *old_interface, void *new_interface)
{
    printf("bs->reinstall_protocol_interface(%p, %s, %p, %p)\n",
        handle, guid_to_ascii(protocol), old_interface, new_interface);

    if (handle == NULL || protocol == NULL)
        return EFI_INVALID_PARAMETER;

    // Make sure the handle has the old_interface
    uemu_protocol *uprotocol = find_uprotocol((uemu_handle *) handle, protocol);
    if (!uprotocol || uprotocol->interface != old_interface)
        return EFI_NOT_FOUND;

    // Replace interface
    uprotocol->interface = new_interface;
    return EFI_SUCCESS;
}

efi_status efiapi uemu_uninstall_protocol_interface(efi_handle handle,
    efi_guid *protocol, void *interface)
{
    printf("bs->uninstall_protocol_inteface(%p, %s, %p)\n",
        handle, guid_to_ascii(protocol), interface);

    if (handle == NULL || protocol == NULL)
        return EFI_INVALID_PARAMETER;

    // ???: should this care whether or not the interface pointer is correct,
    // or should we only care about the GUID
    if (!remove_uprotocol((uemu_handle *) handle, protocol))
        return EFI_NOT_FOUND;

    return EFI_SUCCESS;
}

efi_status efiapi uemu_handle_protocol(efi_handle handle, efi_guid *protocol,
    void **interface)
{
    printf("bs->handle_protocol(%p, %s)\n", handle, guid_to_ascii(protocol));

    // Invalid handle, this error code is in the spec
    if (handle == NULL)
        return EFI_INVALID_PARAMETER;

    // Search for protocol
    uemu_protocol *proto = ((uemu_handle *) handle)->protocols;
    while (proto) {
        if (memcmp(proto->guid, protocol, sizeof *protocol) == 0) {
            *interface = proto->interface;
            return EFI_SUCCESS;
        }
        proto = proto->next;
    }

    // No protocol with the requested GUID, error code not specified,
    // but TianoCore also returns this error code for the same condition
    return EFI_UNSUPPORTED;
}

efi_status efiapi uemu_locate_handle(efi_locate_search_type search_type,
    efi_guid *protocol, void *search_key, efi_size *buffer_size,
    efi_handle *buffer)
{
    printf("bs->locate_handle(%d, %s)\n", search_type, guid_to_ascii(protocol));

    if (search_type == by_register_notify)
        return EFI_UNSUPPORTED;

    // Cound how many handles have the specified protocol
    efi_size handle_cnt = 0;
    for (uemu_handle *cur = uhandle_head; cur; cur = cur->next) {
        if (search_type == all_handles || find_uprotocol(cur, protocol)) {
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

    // Copy all matching handles to the buffer
    efi_size handle_idx = 0;
    for (uemu_handle *cur = uhandle_head; cur; cur = cur->next) {
        if (search_type == all_handles || find_uprotocol(cur, protocol)) {
            buffer[handle_idx++] = (efi_handle) cur;
        }
    }
    return EFI_SUCCESS;
}

efi_status efiapi uemu_locate_protocol(efi_guid *protocol, void *registration,
    void **interface)
{
    printf("bs->locate_protocol(%s)\n", guid_to_ascii(protocol));

    // FIXME: support protocol lookup based on registration
    if (registration != NULL) {
        printf("WARN: bs->locate_protocol: registration != NULL");
        return EFI_UNSUPPORTED;
    }

    for (uemu_handle *cur = uhandle_head; cur; cur = cur->next) {
        uemu_protocol *uprotocol = cur->protocols;
        while (uprotocol) {
            if (memcmp(uprotocol->guid, protocol, sizeof *protocol) == 0) {
                *interface = uprotocol->interface;
                return EFI_SUCCESS;
            }
            uprotocol = uprotocol->next;
        }
    }

    return EFI_NOT_FOUND;
}

efi_status efiapi uemu_open_protocol(efi_handle handle, efi_guid *protocol,
    void **interface, efi_handle agent_handle, efi_handle controller_handle,
    efi_u32 attrib)
{
    printf("WARN: bs->open_protocol() not supported!\n");
    return EFI_UNSUPPORTED;
}

efi_status efiapi uemu_close_protocol(efi_handle handle, efi_guid *protocol,
    efi_handle agent_handle, efi_handle controller_handle)
{
    printf("WARN: bs->close_protocol() not supported!\n");
    return EFI_UNSUPPORTED;
}

efi_status efiapi uemu_open_protocol_information(efi_handle handle,
    efi_guid *protocol, efi_open_protocol_information_entry **entry_buffer,
    efi_size *entry_count)
{
    printf("WARN: bs->open_protocol_information() not supported!\n");
    return EFI_UNSUPPORTED;
}
