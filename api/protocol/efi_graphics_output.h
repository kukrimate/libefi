/*
 * EFI graphics output protocol
 */

#ifndef EFI_GRAPHICS_OUTPUT_H
#define EFI_GRAPHICS_OUTPUT_H

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  { 0x9042a9de, 0x23dc, 0x4a38, 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a }

typedef enum {
  EFI_PIXEL_FORMAT_RGB8,
  EFI_PIXEL_FORMAT_BGR8,
  EFI_PIXEL_FORMAT_BIT_MASK,
  EFI_PIXEL_FORMAT_BLT_ONLY,
  EFI_PIXEL_FORMAT_MAX
} efi_graphics_pixel_format_t;

typedef struct {
  efi_u32_t red_mask;
  efi_u32_t green_mask;
  efi_u32_t blue_mask;
  efi_u32_t reserved_mask;
} efi_pixel_bitmask_t;

typedef struct {
  efi_u32_t version;
  efi_u32_t horizontal_resolution;
  efi_u32_t vertical_resolution;
  efi_graphics_pixel_format_t pixel_format;
  efi_pixel_bitmask_t pixel_information;
  efi_u32_t pixels_per_scan_line;
} efi_graphics_output_mode_information_t;

typedef struct {
  efi_u8_t blue;
  efi_u8_t green;
  efi_u8_t red;
  efi_u8_t reserved;
} efi_graphics_output_blt_pixel_t;

typedef enum {
  EFI_BLT_VIDEO_FILL,
  EFI_BLT_VIDEO_TO_BUFFER,
  EFI_BLT_BUFFER_TO_VIDEO,
  EFI_BLT_VIDEO_TO_VIDEO,
  EFI_GRAPHICS_OUTPUT_BLT_OPERATION_MAX
} efi_graphics_output_blt_operation_t;

typedef struct {
  efi_u32_t max_mode;
  efi_u32_t mode;
  efi_graphics_output_mode_information_t *info;
  efi_size_t size_of_info;
  efi_physical_address_t frame_buffer_base;
  efi_size_t frame_buffer_size;
} efi_graphics_output_protocol_mode_t;

typedef struct efi_graphics_output_protocol efi_graphics_output_protocol_t;

struct efi_graphics_output_protocol {
  efi_status_t (efiapi *query_mode)(efi_graphics_output_protocol_t *self,
                                    efi_u32_t mode_number,
                                    efi_size_t *size_of_info,
                                    efi_graphics_output_mode_information_t **info);
  efi_status_t (efiapi *set_mode)(efi_graphics_output_protocol_t *self,
                                    efi_u32_t mode_number);
  efi_status_t (efiapi *blt)(efi_graphics_output_protocol_t *self,
    efi_graphics_output_blt_pixel_t *blt_buffer,
    efi_graphics_output_blt_operation_t blt_operation,
    efi_size_t source_x, efi_size_t source_y,
    efi_size_t destination_x, efi_size_t desitnation_y,
    efi_size_t width, efi_size_t height,
    efi_size_t delta);
  efi_graphics_output_protocol_mode_t *mode;
};

#endif
