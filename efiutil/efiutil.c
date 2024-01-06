/*
 * UEFI helper functions
 */
#include <efi.h>
#include <efiutil.h>

efi_handle_t self_image_handle;
efi_system_table_t *st;
efi_boot_services_t *bs;
efi_runtime_services_t *rt;

void efi_init(efi_handle_t image_handle, efi_system_table_t *system_table)
{
	self_image_handle = image_handle;
	st = system_table;
	bs = system_table->boot_services;
	rt = system_table->runtime_services;
}

void efi_abort(efi_ch16_t *error_msg, efi_status_t status)
{
	efi_print(error_msg);
	bs->exit(self_image_handle, status, 0, NULL);

	/* We can't do much if exit fails */
	for (;;)
		;
}

void efi_assert(efi_bool_t condition, efi_ch16_t *error_msg)
{
	if (!condition)
		efi_abort(error_msg, EFI_ABORTED);
}

void *efi_alloc(efi_size_t size)
{
	efi_status_t status;
	void *buffer;

	status = bs->allocate_pool(EFI_LOADER_DATA, size, &buffer);
	if (EFI_ERROR(status))
		efi_abort(L"Cannot allocate memory!\n", status);
	return buffer;
}

void efi_free(void *buffer)
{
	bs->free_pool(buffer);
}

void *efi_realloc(void *oldptr, efi_size_t oldsize, efi_size_t newsize)
{
	void *newptr;

	/* Allocate the new region */
	newptr = efi_alloc(newsize);

	/* Check if is oldptr is NULL */
	if (oldptr == NULL) {
		return newptr;
	}

	/* Copy over the contents from the old to the new region */
	memcpy(newptr, oldptr, oldsize);

	/* Free the old memory region */
	efi_free(oldptr);

	/* Return the pointer to the new memory region */
	return newptr;
}

efi_ssize_t efi_strcmp(efi_ch16_t *str1, efi_ch16_t *str2)
{
    while (*str1 == *str2++) {
        if (*str1++ == 0) {
            return 0;
        }
    }
    return *str1 - str2[-1];
}

efi_size_t efi_strlen(efi_ch16_t *str)
{
	efi_ch16_t *p = str;
	for (; *p; ++p);
	return p - str;
}

efi_size_t efi_strsize(efi_ch16_t *str)
{
	return (efi_strlen(str) + 1) * sizeof(efi_ch16_t);
}

// Length of an EFI device path node
#define dp_node_len(node)  (node->length[0] | (node->length[1] << 8))
#define dp_next_node(node) ((efi_device_path_protocol_t *) (((efi_u8_t *) node) + dp_node_len(node)))
#define dp_add_pointer(x, y) (efi_device_path_protocol_t *) ((efi_size_t) x + (efi_size_t) y)

// Determine the length of an EFI device path
static efi_size_t get_dp_len(efi_device_path_protocol_t *dp)
{
	efi_device_path_protocol_t *first;

	first = dp;

	while (dp->type != EFI_END_DEVICE_PATH_TYPE) {
		dp = dp_next_node(dp);
	}

	return (efi_size_t) dp - (efi_size_t) first;
}

static void fill_end_dp_node(efi_device_path_protocol_t *end_dp)
{
	end_dp->type = EFI_END_DEVICE_PATH_TYPE;
	end_dp->sub_type = EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE;
	end_dp->length[0] = sizeof(efi_device_path_protocol_t);
	end_dp->length[1] = 0;
}

static void fill_file_path_dp_node(efi_filepath_device_path_t *node, efi_ch16_t *str, efi_size_t len)
{
	node->header.type = EFI_MEDIA_DEVICE_PATH;
	node->header.sub_type = EFI_MEDIA_FILEPATH_DEVICE_PATH;
	node->header.length[0] = sizeof(efi_device_path_protocol_t) + len;
	node->header.length[1] = 0;
	memcpy(node->path_name, str, len);
}

efi_device_path_protocol_t *merge_device_paths(efi_device_path_protocol_t *first,
	efi_device_path_protocol_t *second)
{
	efi_size_t first_len;
	efi_size_t second_len;
	efi_device_path_protocol_t *result;

	first_len = get_dp_len(first);
	second_len = get_dp_len(second);

	result = efi_alloc(first_len + second_len + sizeof(efi_device_path_protocol_t));

	// Copy the contents of the device paths
	memcpy(result, first, first_len);
	memcpy(dp_add_pointer(result, first_len), second, second_len);

	// Create the end node for the resulting device path
	fill_end_dp_node(dp_add_pointer(result, (first_len + second_len)));

	// Return
	return result;
}

efi_device_path_protocol_t *append_efi_filepath_device_path_t(
	efi_device_path_protocol_t *base, efi_ch16_t *file_path)
{
	efi_device_path_protocol_t *result;
	efi_size_t base_len, file_path_len;

	base_len = get_dp_len(base);
	file_path_len = efi_strsize(file_path);

	result = efi_alloc(base_len + 2 * sizeof(efi_device_path_protocol_t) + file_path_len);

	memcpy(result, base, base_len);
	fill_file_path_dp_node((efi_filepath_device_path_t *) dp_add_pointer(result, base_len), file_path, file_path_len);
	fill_end_dp_node(dp_add_pointer(result, (base_len + sizeof(efi_device_path_protocol_t) + file_path_len)));

	return result;
}

efi_status_t locate_all_handles(efi_guid_t *protocol, efi_size_t *num_handles,
	efi_handle_t **out_buffer)
{
#ifdef USE_EFI110
	return bs->locate_handle_buffer(EFI_LOCATE_BY_PROTOCOL, protocol, NULL, num_handles, out_buffer);
#else
	efi_status_t status;
	efi_size_t buffer_size;

	buffer_size = sizeof(efi_handle_t);
retry:
	*out_buffer = efi_alloc(buffer_size);

	status = bs->locate_handle(by_protocol, protocol, NULL, &buffer_size, *out_buffer);
	if (status == EFI_BUFFER_TOO_SMALL) {
		efi_free(*out_buffer);
		goto retry;
	}
	if (EFI_ERROR(status)) {
		return status;
	}
	*num_handles = buffer_size / sizeof(efi_handle_t);
	return status;
#endif
}

efi_status_t locate_protocol(efi_guid_t *protocol, void **iface)
{
#ifdef USE_EFI110
	return bs->locate_protocol(protocol, NULL, iface);
#else
	efi_status_t status;
	efi_size_t handle_cnt;
	efi_handle_t *handle_buf;

	handle_buf = NULL;
	status = locate_all_handles(protocol, &handle_cnt, &handle_buf);
	if (EFI_ERROR(status))
		goto done;
	if (!handle_cnt) {
		status = EFI_NOT_FOUND;
		goto done;
	}

	status = bs->handle_protocol(handle_buf[0], protocol, iface);

done:
	if (handle_buf)
		efi_free(handle_buf);

	return status;
#endif
}

efi_status_t get_file_info(efi_file_protocol_t *file, efi_file_info_t **file_info)
{
	efi_status_t status;
	efi_size_t bufsize;

	bufsize = 0;
	*file_info = NULL;
retry:
	status = file->get_info(file,
		&(efi_guid_t) EFI_FILE_INFO_ID,
		&bufsize,
		*file_info);

	if (status == EFI_BUFFER_TOO_SMALL) {
		*file_info = efi_alloc(bufsize);
		goto retry;
	}

	return status;
}

efi_status_t efi_read_file(efi_handle_t device_handle, efi_ch16_t *file_path,
	efi_size_t *out_size, void **out_data)
{
	efi_status_t status = EFI_SUCCESS;
	efi_simple_file_system_protocol_t *file_system = NULL;
	efi_file_protocol_t *volume_file = NULL, *file = NULL;
	efi_file_info_t *file_info = NULL;

	status = locate_protocol(&(efi_guid_t) EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void **) &file_system);
	if (status != EFI_SUCCESS)
		goto out;

	status = file_system->open_volume(file_system, &volume_file);
	if (status != EFI_SUCCESS)
		goto out;

	status = volume_file->open(volume_file, &file, file_path, EFI_FILE_MODE_READ, 0);
	if (status != EFI_SUCCESS)
		goto out;

	status = get_file_info(file, &file_info);
	if (status != EFI_SUCCESS)
		goto out;

	*out_size = file_info->file_size;
	*out_data = efi_alloc(*out_size);

	status = file->read(file, out_size, *out_data);
	if (status != EFI_SUCCESS)
		efi_free(*out_data);

out:
	if (file_info)
		efi_free(file_info);
	if (file)
		file->close(file);
	if (volume_file)
		volume_file->close(volume_file);
	return status;
}
