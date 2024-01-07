/*
 * Linux loader
 */

#include <efi.h>
#include <efiutil.h>
#include "bootparam.h"

#define PAGE_SIZE 4096
#define PAGE_COUNT(x) ((x + PAGE_SIZE - 1) / PAGE_SIZE)

static efi_status_t alloc_aligned(efi_size_t alingment, efi_size_t bytes, void **buffer)
{
  efi_status_t status;
  efi_size_t off;

  status = efi_bs->allocate_pages(
    EFI_ALLOCATE_ANY_PAGES,
    EFI_LOADER_CODE,
    PAGE_COUNT(alingment + bytes),
    (efi_physical_address_t *) buffer);

  if (EFI_ERROR(status))
    return status;

  *buffer += alingment - (efi_size_t) *buffer % alingment;
  return status;
}

static efi_status_t convert_mmap(struct boot_params *boot_params, efi_size_t *map_key)
{
  efi_status_t status;
  efi_loaded_image_protocol_t *loaded_image;

  /* UEFI memory map */
  void    *mmap;
  efi_size_t  mmap_size;
  efi_size_t  desc_size;
  efi_u32_t   desc_ver;
  efi_memory_descriptor_t *mmap_ent;

  /* E820 memory map */
  efi_u32_t   e820_last_type;
  efi_size_t  e820_entries;
  struct boot_e820_entry  *e820_cur;

  status = efi_bs->handle_protocol(
    efi_image_handle,
    &(efi_guid_t) EFI_LOADED_IMAGE_PROTOCOL_GUID,
    (void **) &loaded_image);
  if (EFI_ERROR(status))
    return status;

  mmap = NULL;
  mmap_size = 0;

retry:
  status = efi_bs->get_memory_map(
    &mmap_size,
    mmap,
    map_key,
    &desc_size,
    &desc_ver);

  if (status == EFI_BUFFER_TOO_SMALL) {
    mmap = efi_alloc(mmap_size);
    goto retry;
  }

  if (EFI_ERROR(status)) /* Failed to get UEFI memory map */
    return status;

  /* Allocate an E820 memory map with the same number of entries */
  e820_cur = boot_params->e820_table;
  e820_entries = mmap_size / desc_size;
  /* Make sure this firmware's memory map is compatible with the kernel */
  if (e820_entries > E820_MAX_ENTRIES_ZEROPAGE) {
    efi_free(mmap);
    return EFI_UNSUPPORTED;
  }
  boot_params->e820_entries = e820_entries;

  /* Convert UEFI memory map to E820 */
  e820_last_type = 0;
  for (mmap_ent = mmap; (void *) mmap_ent < mmap + mmap_size;
        mmap_ent = (void *) mmap_ent + desc_size) {
    e820_cur->addr = mmap_ent->start;
    e820_cur->size = mmap_ent->number_of_pages * PAGE_SIZE;

    switch (mmap_ent->type) {
    case EFI_CONVENTIONAL_MEMORY:
    case EFI_LOADER_CODE:
    case EFI_LOADER_DATA:
    case EFI_BOOT_SERVICES_CODE:
    case EFI_BOOT_SERVICES_DATA:
      e820_cur->type = E820_USABLE;
      break;
    case EFI_ACPI_RECLAIM_MEMORY:
      e820_cur->type = E820_ACPI_RECLAIM;
      break;
    case EFI_ACPI_MEMORY_NVS:
      e820_cur->type = E820_APCI_NVS;
      break;
    case EFI_UNUSABLE_MEMORY:
      e820_cur->type = E820_UNUSABLE;
      break;
    /* Default to reserved */
    default:
      e820_cur->type = E820_RESERVED;
      break;
    }

    /* Merge subsequent regions with the same type */
    if (e820_cur->type == e820_last_type &&
        e820_cur[-1].addr +
        e820_cur[-1].size == e820_cur->addr) {
      e820_cur[-1].size += e820_cur->size;
      --boot_params->e820_entries;
    } else {
      e820_last_type = e820_cur->type;
      ++e820_cur;
    }
  }

  /* NOTE: the UEFI memmap cannot ever be freed otherwise calling
     ExitBootServices is not allowed */
  return status;
}

static efi_status_t setup_video(struct boot_params *boot_params)
{
  efi_status_t  status;

  efi_graphics_output_protocol_t *gop;
  efi_graphics_output_mode_information_t *mode_info;

  status = efi_locate_protocol(
    &(efi_guid_t) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
    (void **) &gop);
  if (EFI_ERROR(status))
    goto done;

  mode_info = gop->mode->info;
  switch (mode_info->pixel_format) {
  case EFI_PIXEL_FORMAT_RGB8:
    boot_params->screen_info.lfb_depth = 32;
    boot_params->screen_info.red_size = 8;
    boot_params->screen_info.red_pos = 0;
    boot_params->screen_info.green_size = 8;
    boot_params->screen_info.green_pos = 8;
    boot_params->screen_info.blue_size = 8;
    boot_params->screen_info.blue_pos = 16;
    boot_params->screen_info.rsvd_size = 8;
    boot_params->screen_info.rsvd_pos = 24;
    boot_params->screen_info.lfb_linelength =
      mode_info->pixels_per_scan_line * 4;
    break;
  case EFI_PIXEL_FORMAT_BGR8:
    boot_params->screen_info.lfb_depth = 32;
    boot_params->screen_info.red_size = 8;
    boot_params->screen_info.red_pos = 16;
    boot_params->screen_info.green_size = 8;
    boot_params->screen_info.green_pos = 8;
    boot_params->screen_info.blue_size = 8;
    boot_params->screen_info.blue_pos = 0;
    boot_params->screen_info.rsvd_size = 8;
    boot_params->screen_info.rsvd_pos = 24;
    boot_params->screen_info.lfb_linelength =
      mode_info->pixels_per_scan_line * 4;
    break;
  default: /* TODO: add more pixel formats */
    status = EFI_UNSUPPORTED;
    goto done;
  }

  boot_params->screen_info.orig_video_isVGA = 0x70; /* EFI framebuffer */
  boot_params->screen_info.lfb_base =
    (efi_u32_t) (efi_u64_t) gop->mode->frame_buffer_base;
  boot_params->screen_info.ext_lfb_base =
    (efi_u32_t) ((efi_u64_t) gop->mode->frame_buffer_base >> 32);
  boot_params->screen_info.lfb_size = gop->mode->frame_buffer_size;
  boot_params->screen_info.lfb_width = mode_info->horizontal_resolution;
  boot_params->screen_info.lfb_height = mode_info->vertical_resolution;
  boot_params->screen_info.pages = 1;

done:
  return status;
}

static efi_status_t locate_self_volume(efi_simple_file_system_protocol_t **self_volume)
{
  efi_status_t status;
  efi_loaded_image_protocol_t *loaded_image;

  status = efi_bs->handle_protocol(
    efi_image_handle,
    &(efi_guid_t) EFI_LOADED_IMAGE_PROTOCOL_GUID,
    (void **) &loaded_image);
  if (EFI_ERROR(status))
    return status;

  status = efi_bs->handle_protocol(
    loaded_image->device_handle,
    &(efi_guid_t) EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
    (void **) self_volume);
  return status;
}

static efi_status_t read_file(efi_file_protocol_t *file, efi_ssize_t offs, efi_size_t size, void *buffer)
{
  efi_status_t status;
  efi_size_t outsize;

  if (offs >= 0) {
    status = file->set_position(file, offs);
    if (EFI_ERROR(status))
      return status;
  }

  outsize = size;
  status = file->read(file, &outsize, buffer);
  if (EFI_ERROR(status))
    return status;

  return status;
}

static efi_status_t get_file_size(efi_file_protocol_t *file, efi_size_t *file_size)
{
  efi_status_t  status;
  efi_file_info_t *file_info;

  status = efi_get_file_info(file, &file_info);
  if (EFI_ERROR(status))
    return status;

  *file_size = file_info->file_size;
  efi_free(file_info);
  return status;
}

static efi_status_t boot_linux(efi_ch16_t *kernel_path, efi_ch16_t *initrd_path, char *cmdline)
{
  efi_status_t    status;

  efi_size_t    cmdline_size;
  struct boot_params  *boot_params;

  efi_simple_file_system_protocol_t *volume;
  efi_file_protocol_t   *root_dir;

  efi_file_protocol_t *kernel_file;
  void      *kernel_base;

  efi_file_protocol_t *initrd_file;
  efi_size_t    initrd_size;
  void      *initrd_base;

  efi_size_t    map_key;

  /* Allocate boot params + cmdline buffer */
  cmdline_size = strlen(cmdline) + 1;
  status = efi_bs->allocate_pages(
    EFI_ALLOCATE_ANY_PAGES,
    EFI_LOADER_DATA,
    PAGE_COUNT(sizeof(struct boot_params) + cmdline_size),
    (efi_physical_address_t *) &boot_params);
  if (EFI_ERROR(status))
    return status;
  efi_print(L"Boot params at %p\n", boot_params);
  /* Zero boot params */
  memset(boot_params, 0, sizeof(struct boot_params));
  /* Copy cmdline */
  memcpy(&boot_params[1], cmdline, cmdline_size);

  /* Open boot volume */
  status = locate_self_volume(&volume);
  if (EFI_ERROR(status))
    goto err_free_boot_params;
  status = volume->open_volume(volume, &root_dir);
  if (EFI_ERROR(status))
    goto err_free_boot_params;

  /* Open kernel */
  status = root_dir->open(
    root_dir,
    &kernel_file,
    kernel_path,
    EFI_FILE_MODE_READ,
    0);
  if (EFI_ERROR(status))
    goto err_close_rootdir;

  /* Read setup header */
  status = read_file(kernel_file,
    0x1f1,
    sizeof(struct setup_header),
    &boot_params->hdr);
  if (EFI_ERROR(status))
    goto err_close_kernel;

  /* Enforce all assumptions made about the kernel image */
  if (boot_params->hdr.boot_flag != 0xaa55 ||
      boot_params->hdr.header != 0x53726448 ||
      boot_params->hdr.version < 0x02c ||
      !boot_params->hdr.relocatable_kernel ||
      !(boot_params->hdr.xloadflags & XLF_KERNEL_64) ||
      !(boot_params->hdr.xloadflags
        & XLF_CAN_BE_LOADED_ABOVE_4G)) {
    status = EFI_INVALID_PARAMETER;
    goto err_close_kernel;
  }

  /* Allocate buffer for the kernel image */
  efi_print(L"Kernel alingment: %p\n", boot_params->hdr.kernel_alignment);
  status = alloc_aligned(
    boot_params->hdr.kernel_alignment,
    boot_params->hdr.init_size,
    &kernel_base);
  if (EFI_ERROR(status))
    goto err_close_kernel;
  efi_print(L"Kernel will be loaded at: %p\n", kernel_base);

  /* Load kernel */
  status = read_file(kernel_file,
    (boot_params->hdr.setup_sects + 1) * 512,
    boot_params->hdr.init_size,
    kernel_base);
  if (EFI_ERROR(status))
    goto err_free_kernel;

  /*
   * Load initrd
   */
  status = root_dir->open(
    root_dir,
    &initrd_file,
    initrd_path,
    EFI_FILE_MODE_READ,
    0);
  if (EFI_ERROR(status))
    goto err_free_kernel;

  status = get_file_size(initrd_file, &initrd_size);
  if (EFI_ERROR(status))
    goto err_close_initrd;

  status = efi_bs->allocate_pages(
    EFI_ALLOCATE_ANY_PAGES,
    EFI_LOADER_DATA,
    PAGE_COUNT(initrd_size),
    (efi_physical_address_t *) &initrd_base);
  if (EFI_ERROR(status))
    goto err_close_initrd;

  status = read_file(initrd_file, -1, initrd_size, initrd_base);
  if (EFI_ERROR(status))
    goto err_free_initrd;

  /* Now we can close all file handles */
  initrd_file->close(initrd_file);
  kernel_file->close(kernel_file);
  root_dir->close(root_dir);

  boot_params->hdr.type_of_loader = 0xff;
  /* Make sure the kernel is *not* quiet */
  boot_params->hdr.loadflags &= ~(1 << 5);
  /* The command line is in the same buffer right after the bootparams */
  boot_params->hdr.cmd_line_ptr = (efi_u64_t) &boot_params[1];
  boot_params->ext_cmd_line_ptr = (efi_u64_t) &boot_params[1] >> 32;
  /* Set initrd address and size */
  boot_params->hdr.ramdisk_image = (efi_u64_t) initrd_base;
  boot_params->ext_ramdisk_image = (efi_u64_t) initrd_base >> 32;
  boot_params->hdr.ramdisk_size = (efi_u64_t) initrd_size;
  boot_params->ext_ramdisk_size = (efi_u64_t) initrd_size >> 32;

  /* Find ACPI RSDP */
  for (size_t i = 0; i < efi_st->cnt_config_entries; ++i)
    if (!memcmp(&efi_st->config_entries[i].vendor_guid,
        &(efi_guid_t) EFI_ACPI_TABLE_GUID,
        sizeof(efi_guid_t)))
      boot_params->acpi_rsdp_addr =
        (efi_size_t) efi_st->config_entries[i].vendor_table;

  /* Find the framebuffer */
  status = setup_video(boot_params);
  if (EFI_ERROR(status))
    efi_print(L"WARN: graphics setup failed!\n");

  /* Convert the UEFI memory map to E820 for the kernel */
  status = convert_mmap(boot_params, &map_key);
  if (EFI_ERROR(status))
    return status;
  /* Get rid of boot services */
  status = efi_bs->exit_boot_services(efi_image_handle, map_key);
  if (EFI_ERROR(status))
    return status;

  /* Jump to the kernel's entry point */
  asm volatile (
    "cli\n"
    "movq %0, %%rax\n"
    "movq %1, %%rsi\n"
    "jmp *%%rax" ::
    "g" (kernel_base + 0x200),
    "g" (boot_params)
    : "rax", "rsi");

err_free_initrd:
  efi_bs->free_pages((efi_physical_address_t) initrd_base,
    PAGE_COUNT(initrd_size));
err_close_initrd:
  initrd_file->close(initrd_file);
err_free_kernel:
  efi_bs->free_pages((efi_physical_address_t) kernel_base,
    PAGE_COUNT(boot_params->hdr.init_size));
err_close_kernel:
  kernel_file->close(kernel_file);
err_close_rootdir:
  root_dir->close(root_dir);
err_free_boot_params:
  efi_bs->free_pages(
    (efi_physical_address_t) boot_params,
    PAGE_COUNT(sizeof(struct boot_params) + cmdline_size));
  return status;
}

efi_status_t
efiapi
efi_main(efi_handle_t image_handle, efi_system_table_t *system_table)
{
  efi_status_t status;

  efi_init(image_handle, system_table);
  efi_print(L"libefi loadlin %s\n", GIT_REV);

  status = boot_linux(
    L"vmlinuz-4.19.0-10-amd64",
    L"initrd.img-4.19.0-10-amd64",
    "root=UUID=b2e1c499-2f97-4f0b-a3a6-d356dab64705 rw nokaslr");
  return status;
}
