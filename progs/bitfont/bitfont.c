#include <efi.h>
#include <efiutil.h>
#include "fbterm.h"

/* GOP only uses one specific pixel width */
#define EFI_GOP_PIXELWIDTH 4

/*
 * Setup an fbinfo struct from UEFI GOP
 */
static efi_status_t gop_to_fbinfo(struct fbinfo *fb)
{
	efi_status_t status;
	efi_graphics_output_protocol_t *gop;

	status = locate_protocol(
		&(efi_guid_t) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
		(void **) &gop);
	if (EFI_ERROR(status))
		goto done;

	switch (gop->mode->info->pixel_format) {
	case EFI_PIXEL_FORMAT_RGB8:
		fb->red_idx = 0;
		fb->grn_idx = 1;
		fb->blu_idx = 2;
		break;
	case EFI_PIXEL_FORMAT_BGR8:
		fb->red_idx = 2;
		fb->grn_idx = 1;
		fb->blu_idx = 0;
		break;
	default:
		status = EFI_UNSUPPORTED;
		goto done;
	}

	fb->base = gop->mode->frame_buffer_base;
	fb->x_limit = gop->mode->info->horizontal_resolution;
	fb->y_limit = gop->mode->info->vertical_resolution;
	fb->linewidth = gop->mode->info->pixels_per_scan_line * EFI_GOP_PIXELWIDTH;
	fb->pixelwidth = EFI_GOP_PIXELWIDTH;

done:
	return status;
}

efi_status_t efiapi efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
	efi_status_t status;
	struct fbinfo fb;
	efi_size_t index;

	efi_init(image_handle, system_table);

	status = gop_to_fbinfo(&fb);
	if (EFI_ERROR(status))
		return status;

	/* Clear the screen */
	fb_clear(&fb, 0, 0, 0);

	for (size_t i = 0; i < 10000; ++i)
		fb_print(&fb, "Hello, World %ld!\n", i);

	bs->wait_for_event(1, &st->con_in->wait_for_key, &index);
	return status;
}
