#ifndef __EFIUTIL_H
#define __EFIUTIL_H

// globals
extern efi_handle self_image_handle;
extern efi_loaded_image_protocol *self_loaded_image;
extern efi_simple_file_system_protocol *self_volume;
extern efi_file_protocol *self_root_dir;
extern efi_system_table *st;
extern efi_boot_services *bs;

/* Initialize and deinitialize util structures */
void init_util(efi_handle image_handle, efi_system_table *system_table);
void fini_util();

/* Simple printf like function */
void print(efi_ch16 *format, ...);

/* Exit with status after printing error_msg */
void abort(efi_ch16 *error_msg, efi_status status);

/* Zero a len sized memory region pointed to by buffer */
void bzero(void *buffer, efi_size len);

/* Copy n bytes from src to dest */
void memcpy(void *dest, void *src, efi_size n);

/* Allocate a size sized memory region */
void *malloc(efi_size size);

/* Free the memory region pointed to by buffer */
void free(void *buffer);

/* Allocate a newsize sized memory region and copy over oldsize bytes
 * from oldptr and free the region pointed to by oldptr
 * if oldptr is null just allocate the new region and return */
void *realloc(void *oldptr, efi_size oldsize, efi_size newsize);

/* Determine the length of an EFI string pointed to by str */
efi_size strlen(efi_ch16 *str);

/* Determine how many a bytes an EFI string takes to store
 *  including the null-terminator */
efi_size strsize(efi_ch16 *str);

/* Determine the length of an ASCII string pointed to by str */
efi_size ascii_strlen(char *str);

/* Allocate a buffer to store the device paths pointed first and second and merge them */
efi_device_path_protocol *merge_device_paths(efi_device_path_protocol *first, efi_device_path_protocol *second);

/* Generate a file path device path from a string file path and merge it
 * with the device path pointed to by base */
efi_device_path_protocol *append_filepath_device_path(efi_device_path_protocol *base, efi_ch16 *file_path);

/* Locate all EFI handles that support the specified protocol */
efi_status locate_all_handles(efi_guid *protocol, efi_size *num_handles, efi_handle **out_buffer);

#endif
