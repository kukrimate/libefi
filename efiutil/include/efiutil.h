/*
 * UEFI dependent helper functions
 */
#ifndef EFIUTIL_H
#define EFIUTIL_H

/*
 * efiutil provides some standard library support
 */
#include <stdarg.h>
#include <string.h>

/*
 * Globals for accessing UEFI
 */
extern efi_handle_t efi_image_handle;
extern efi_system_table_t *efi_st;
extern efi_boot_services_t *efi_bs;
extern efi_runtime_services_t *efi_rt;

/*
 * Must be called before using the globals above
 */
void efi_init(efi_handle_t image_handle, efi_system_table_t *system_table);

/*
 * Like printf, but uses UEFI strings
 */
void efi_print(efi_ch16_t *format, ...);

/*
 * Print error_msg, then exit with status
 */
void efi_abort(efi_ch16_t *error_msg, efi_status_t status);

/*
 * Assert that condition is true, print error_msg and abort if it fails
 */
void efi_assert(efi_bool_t condition, efi_ch16_t *error_msg);

/*
 * Allocate an n byte memory region
 */
void *efi_alloc(efi_size_t n);

/*
 * Free the memory region pointed to by ptr
 */
void efi_free(void *ptr);

/*
 * Resize a memory region from oldsize to newsize
 */
void *efi_realloc(void *oldptr, efi_size_t oldsize, efi_size_t newsize);

/*
 * Compare two EFI strings for equality
 * The return value works similar to strcmp
 */
efi_ssize_t efi_strcmp(efi_ch16_t *str1, efi_ch16_t *str2);

/*
 * Determine the length of an EFI string pointed to by str
 */
efi_size_t efi_strlen(efi_ch16_t *str);

/*
 * Determine how many a bytes an EFI string takes to store
 *  including the null-terminator
 */
efi_size_t efi_strsize(efi_ch16_t *str);

/*
 * Merge two device path instances
 */
efi_device_path_protocol_t *efi_dp_merge(
    efi_device_path_protocol_t *first,
    efi_device_path_protocol_t *second);

/*
 * Merge a device path and a file path converted to a device path
 */
efi_device_path_protocol_t *efi_dp_append_file_path(
    efi_device_path_protocol_t *base, efi_ch16_t *file_path);

/*
 * Locate all EFI handles that support the specified protocol
 */
efi_status_t efi_locate_all_handles(efi_guid_t *protocol,
    efi_size_t *num_handles, efi_handle_t **out_buffer);

/*
 * Locate the first instance of a protocol
 */
efi_status_t efi_locate_protocol(efi_guid_t *protocol, void **iface);

/*
 * Get the file info struct for file
 */
efi_status_t efi_get_file_info(efi_file_protocol_t *file,
    efi_file_info_t **file_info);

/*
 * Read a file from a filesytem
 */
efi_status_t efi_read_file(efi_handle_t device_handle, efi_ch16_t *file_path,
    efi_size_t *out_size, void **out_data);

#endif
