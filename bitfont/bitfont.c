#include <efi.h>
#include <efiutil.h>
#include "fbterm.h"

/* GOP only uses one specific pixel width */
#define EFI_GOP_PIXELWIDTH 4

/*
 * Setup an fbinfo struct from UEFI GOP
 */
static
efi_status
gop_to_fbinfo(struct fbinfo *fb)
{
	efi_status status;
	efi_graphics_output_protocol *gop;

	status = locate_protocol(
		&(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
		(void **) &gop);
	if (EFI_ERROR(status))
		goto done;

	gop->set_mode(gop, 11);

	switch (gop->mode->info->pixel_format) {
	case pixel_red_green_blue_reserved_8_bit_per_color:
		fb->red_idx = 0;
		fb->grn_idx = 1;
		fb->blu_idx = 2;
		break;
	case pixel_blue_green_red_reserved_8_bit_per_color:
		fb->red_idx = 2;
		fb->grn_idx = 1;
		fb->blu_idx = 0;
		break;
	default:
		status = EFI_UNSUPPORTED;
		goto done;
	}

	fb->base = gop->mode->frame_buffer_base;
	fb->size = gop->mode->frame_buffer_size;
	fb->x_limit = gop->mode->info->horizontal_resolution;
	fb->y_limit = gop->mode->info->vertical_resolution;
	fb->linewidth = gop->mode->info->pixels_per_scan_line * EFI_GOP_PIXELWIDTH;
	fb->pixelwidth = EFI_GOP_PIXELWIDTH;

done:
	return status;
}

efi_status
efiapi
efi_main(efi_handle image_handle, efi_system_table *system_table)
{
	efi_status status;
	struct fbinfo fb;
	efi_size index;

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
