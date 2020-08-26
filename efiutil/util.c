#include <efi.h>
#include <efiutil.h>

// globals
efi_handle self_image_handle;
efi_loaded_image_protocol *self_loaded_image;
efi_simple_file_system_protocol *self_volume;
efi_file_protocol *self_root_dir;
efi_system_table *st;
efi_boot_services *bs;

void init_util(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;

	self_image_handle = image_handle;
	st = system_table;
	bs = system_table->boot_services;

	// Locate self loaded image
	status = bs->handle_protocol(
		self_image_handle,
		&(efi_guid) EFI_LOADED_IMAGE_PROTOCOL_GUID,
		(void **) &self_loaded_image);
	if (EFI_ERROR(status)) {
		abort(L"Error handling efi_loaded_image_protocol_t for the image handle!\r\n", status);
	}

	// Locate the volume we booted from
	status = bs->handle_protocol(
		self_loaded_image->device_handle,
		&(efi_guid) EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
		(void **) &self_volume);
	if (EFI_ERROR(status)) {
		abort(L"Error locating self volume!\r\n", status);
	}

	// Open the self volumegi
	status = self_volume->open_volume(self_volume, &self_root_dir);
	if (EFI_ERROR(status)) {
		abort(L"Error opening self volume!\r\n", status);
	}
}

void fini_util()
{
	efi_status status;

	status = self_root_dir->close(self_root_dir);
	if (EFI_ERROR(status)) {
		abort(L"Error closing self volume!\r\n", status);
	}
}

void abort(efi_ch16 *error_msg, efi_status status)
{
	print(error_msg);
	bs->exit(self_image_handle, status, 0, NULL);

	// Hang the system if exiting fails
	asm volatile ("cli");
	for (;;) {
		asm volatile("hlt");
	}
}

void bzero(void *buffer, efi_size len)
{
	for (efi_size i = 0; i < len; ++i) {
		((efi_u8 *) buffer)[i] = 0;
	}
}

void memcpy(void *dest, void *src, efi_size n)
{
	for (efi_size i = 0; i < n; ++i) {
		((efi_u8 *) dest)[i] = ((efi_u8 *) src)[i];
	}
}

void *realloc(void *oldptr, efi_size oldsize, efi_size newsize)
{
	void *newptr;

	/* Allocate the new region */
	newptr = malloc(newsize);

	/* Check if is oldptr is NULL */
	if (oldptr == NULL) {
		return newptr;
	}

	/* Copy over the contents from the old to the new region */
	memcpy(newptr, oldptr, oldsize);

	/* Free the old memory region */
	free(oldptr);

	/* Return the pointer to the new memory region */
	return newptr;
}

void *malloc(efi_size size)
{
	void *buffer;
	efi_status status;

	status = bs->allocate_pool(efi_loader_data, size, &buffer);
	if (EFI_ERROR(status)) {
		abort(L"Dynamic memory allocation failed!\r\n", status);
	}

	bzero(buffer, size);

	return buffer;
}

void free(void *buffer)
{
	bs->free_pool(buffer);
}

// String utils
efi_size strlen(efi_ch16 *str)
{
	efi_ch16 *p = str;
	for (; *p; ++p);
	return p - str;
}

efi_size ascii_strlen(char *str)
{
	char *p = str;
	for (; *p; ++p);
	return p - str;
}

efi_size strsize(efi_ch16 *str)
{
	return (strlen(str) + 1) * sizeof(efi_ch16);
}

// Length of an EFI device path node
#define dp_node_len(node)  (node->length[0] | (node->length[1] << 8))
#define dp_next_node(node) ((efi_device_path_protocol *) (((efi_u8 *) node) + dp_node_len(node)))
#define dp_add_pointer(x, y) (efi_device_path_protocol *) ((efi_size) x + (efi_size) y)

// Determine the length of an EFI device path
static efi_size get_dp_len(efi_device_path_protocol *dp)
{
	efi_device_path_protocol *first;

	first = dp;

	while (dp->type != END_DEVICE_PATH_TYPE) {
		dp = dp_next_node(dp);
	}

	return (efi_size) dp - (efi_size) first;
}

static void fill_end_dp_node(efi_device_path_protocol *end_dp)
{
	end_dp->type = END_DEVICE_PATH_TYPE;
	end_dp->sub_type = END_ENTIRE_DEVICE_PATH_SUBTYPE;
	end_dp->length[0] = sizeof(efi_device_path_protocol);
	end_dp->length[1] = 0;
}

static void fill_file_path_dp_node(filepath_device_path *node, efi_ch16 *str, efi_size len)
{
	node->header.type = MEDIA_DEVICE_PATH;
	node->header.sub_type = MEDIA_FILEPATH_DP;
	node->header.length[0] = sizeof(efi_device_path_protocol) + len;
	node->header.length[1] = 0;
	memcpy(node->path_name, str, len);
}

// Merge two EFI device paths together (only the first instances are used)
efi_device_path_protocol *merge_device_paths(efi_device_path_protocol *first, efi_device_path_protocol *second)
{
	efi_size first_len;
	efi_size second_len;
	efi_device_path_protocol *result;

	first_len = get_dp_len(first);
	second_len = get_dp_len(second);

	result = malloc(first_len + second_len + sizeof(efi_device_path_protocol));

	// Copy the contents of the device paths
	memcpy(result, first, first_len);
	memcpy(dp_add_pointer(result, first_len), second, second_len);

	// Create the end node for the resulting device path
	fill_end_dp_node(dp_add_pointer(result, (first_len + second_len)));

	// Return
	return result;
}

// Append a filepath device path node to the end of an EFI device path
efi_device_path_protocol *append_filepath_device_path(efi_device_path_protocol *base, efi_ch16 *file_path)
{
	efi_device_path_protocol *result;
	efi_size base_len, file_path_len;

	base_len = get_dp_len(base);
	file_path_len = strsize(file_path);

	result = malloc(base_len + 2 * sizeof(efi_device_path_protocol) + file_path_len);

	memcpy(result, base, base_len);
	fill_file_path_dp_node((filepath_device_path *) dp_add_pointer(result, base_len), file_path, file_path_len);
	fill_end_dp_node(dp_add_pointer(result, (base_len + sizeof(efi_device_path_protocol) + file_path_len)));

	return result;
}

efi_status locate_all_handles(efi_guid *protocol, efi_size *num_handles, efi_handle **out_buffer)
{
	efi_status status;
	efi_size buffer_size;

	buffer_size = sizeof(efi_handle);
retry:
	*out_buffer = malloc(buffer_size);

	status = bs->locate_handle(by_protocol, protocol, NULL, &buffer_size, *out_buffer);
	if (status == EFI_BUFFER_TOO_SMALL) {
		free(*out_buffer);
		goto retry;
	}
	if (EFI_ERROR(status)) {
		return status;
	}
	*num_handles = buffer_size / sizeof(efi_handle);
	return status;
}

/* File utils */

efi_status
efiapi
get_file_info(efi_file_protocol *file, efi_file_info **file_info)
{
	efi_status status;
	efi_size bufsize;

	bufsize = 0;
	*file_info = NULL;
retry:
	status = file->get_info(file,
		&(efi_guid) EFI_FILE_INFO_ID,
		&bufsize,
		*file_info);

	if (status == EFI_BUFFER_TOO_SMALL) {
		*file_info = malloc(bufsize);
		goto retry;
	}

	return status;
}
