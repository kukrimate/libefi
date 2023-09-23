/*
 * EFI device path protocol
 */

#ifndef EFI_DEVICE_PATH_H
#define EFI_DEVICE_PATH_H

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
  { 0x09576e91, 0x6d3f, 0x11d2, 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }

#define EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID \
  { 0xbc62157e, 0x3e33, 0x4fec, 0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf }

typedef struct efi_device_path_protocol efi_device_path_protocol_t;

struct efi_device_path_protocol {
  efi_u8_t type;
  efi_u8_t sub_type;
  efi_u8_t length[2];
};

//
// Hardware Device Path
//

#define EFI_HARDWARE_DEVICE_PATH              0x01

/* PCI Device Path Sub-type */
#define EFI_HARDWARE_PCI_DEVIE_PATH           0x01

typedef struct {
  efi_device_path_protocol_t header;
  efi_u8_t function;
  efi_u8_t device;
} efi_pci_device_path_t;

/* PCCARD Device Path Sub-type */
#define EFI_HARDWARE_PCCARD_DEVIDE_PATH       0x02

typedef struct {
  efi_device_path_protocol_t header;
  efi_u8_t function;
} efi_pccard_device_path_t;

/* Memory Mapped Device Path Sub-type */
#define EFI_HARDWARE_MMAP_DEVICE_PATH         0x03

typedef struct {
  efi_device_path_protocol_t header;
  efi_memory_type_t memory_type;
  efi_physical_address_t start_address;
  efi_physical_address_t end_address;
} efi_mmap_device_path_t;

/* Vendor-specific Device Path Sub-type */
#define EFI_HARDWARE_VENDOR_DEVICE_PATH       0x04

typedef struct {
  efi_device_path_protocol_t header;
  efi_guid_t vendor_guid;
  // n bytes of vendor specific data
} efi_vendor_device_path_t;

/* Controller device Path Sub-type */
#define EFI_HARDWARE_CONTROLLER_DEVICE_PATH   0x05

typedef struct {
  efi_device_path_protocol_t header;
  efi_u32_t controller_number;
} efi_controller_device_path_t;

//
// ACPI Device Path
//

#define EFI_ACPI_DEVICE_PATH                  0x02

//
// Messaging Device Path
//

#define EFI_MESSAGING_DEVICE_PATH             0x03

//
// Media Device Path
//

#define EFI_MEDIA_DEVICE_PATH                 0x04

/* Hard Drive Device Path Sub-type */
#define EFI_MEDIA_HARDDRIVE_DEVICE_PATH       0x01

#define EFI_MBR_TYPE_PCAT                     0x01
#define EFI_MBR_TYPE_GPT                      0x02

#define EFI_NO_DISK_SIGNATURE                 0x00
#define EFI_SIGNATURE_TYPE_MBR                0x01
#define EFI_SIGNATURE_TYPE_GUID               0x02

typedef struct {
  efi_device_path_protocol_t header;
  efi_u32_t partition_number;
  efi_u64_t partition_start;
  efi_u64_t partition_size;
  efi_u8_t signature[16];
  efi_u8_t mbr_type;
  efi_u8_t signature_type;
} efi_harddrive_device_path_t;

/* CD-ROM Device Path Sub-type */
#define EFI_MEDIA_CDROM_DEVICE_PATH           0x02

typedef struct {
  efi_device_path_protocol_t header;
  efi_u32_t boot_entry;
  efi_u64_t partition_start;
  efi_u64_t partition_size;
} efi_cdrom_device_path_t;

/* Vendor-specific Device Path Sub-type */
#define EFI_MEDIA_VENDOR_DEVICE_PATH          0x03

/* File Path Device Path Sub-type */
#define EFI_MEDIA_FILEPATH_DEVICE_PATH        0x04

typedef struct {
  efi_device_path_protocol_t header;
  efi_ch16_t path_name[1];
} efi_filepath_device_path_t;

/* Media Protocol Device Path Sub-type */
#define EFI_MEDIA_PROTOCOL_DEVICE_PATH        0x05

typedef struct {
  efi_device_path_protocol_t header;
  efi_guid_t protocol;
} efi_media_protocol_device_path_t;

//
// End of Device Path
//

#define EFI_END_DEVICE_PATH_TYPE              0x7f

#define EFI_END_INSTANCE_DEVICE_PATH_SUBTYPE  0x01
#define EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE    0xff

#endif
