#ifndef BOOTPARAM_H
#define BOOTPARAM_H

#define SETUP_NONE                  0
#define SETUP_E820_EXT              1
#define SETUP_DTB                   2
#define SETUP_PCI                   3
#define SETUP_EFI                   4
#define SETUP_APPLE_PROPERTIES      5
#define SETUP_JAILHOUSE             6

#define SETUP_INDIRECT              (1<<31)

#define SETUP_TYPE_MAX              (SETUP_INDIRECT | SETUP_JAILHOUSE)

#define RAMDISK_IMAGE_START_MASK    0x07FF
#define RAMDISK_PROMPT_FLAG         0x8000
#define RAMDISK_LOAD_FLAG           0x4000

#define LOADED_HIGH                 (1<<0)
#define KASLR_FLAG                  (1<<1)
#define QUIET_FLAG                  (1<<5)
#define KEEP_SEGMENTS               (1<<6)
#define CAN_USE_HEAP                (1<<7)

#define XLF_KERNEL_64               (1<<0)
#define XLF_CAN_BE_LOADED_ABOVE_4G  (1<<1)
#define XLF_EFI_HANDOVER_32         (1<<2)
#define XLF_EFI_HANDOVER_64         (1<<3)
#define XLF_EFI_KEXEC               (1<<4)
#define XLF_5LEVEL                  (1<<5)
#define XLF_5LEVEL_ENABLED          (1<<6)

struct setup_data {
  uint64_t next;
  uint32_t type;
  uint32_t len;
  uint8_t data[0];
};

struct setup_indirect {
  uint32_t type;
  uint32_t reserved;
  uint64_t len;
  uint64_t addr;
};

struct setup_header {
  uint8_t setup_sects;
  uint16_t  root_flags;
  uint32_t  syssize;
  uint16_t  ram_size;
  uint16_t  vid_mode;
  uint16_t  root_dev;
  uint16_t  boot_flag;
  uint16_t  jump;
  uint32_t  header;
  uint16_t  version;
  uint32_t  realmode_swtch;
  uint16_t  start_sys_seg;
  uint16_t  kernel_version;
  uint8_t type_of_loader;
  uint8_t loadflags;
  uint16_t  setup_move_size;
  uint32_t  code32_start;
  uint32_t  ramdisk_image;
  uint32_t  ramdisk_size;
  uint32_t  bootsect_kludge;
  uint16_t  heap_end_ptr;
  uint8_t ext_loader_ver;
  uint8_t ext_loader_type;
  uint32_t  cmd_line_ptr;
  uint32_t  initrd_addr_max;
  uint32_t  kernel_alignment;
  uint8_t relocatable_kernel;
  uint8_t min_alignment;
  uint16_t  xloadflags;
  uint32_t  cmdline_size;
  uint32_t  hardware_subarch;
  uint64_t  hardware_subarch_data;
  uint32_t  payload_offset;
  uint32_t  payload_length;
  uint64_t  setup_data;
  uint64_t  pref_address;
  uint32_t  init_size;
  uint32_t  handover_offset;
  uint32_t  kernel_info_offset;
} __attribute__((packed));

struct sys_desc_table {
  uint16_t length;
  uint8_t  table[14];
};

struct olpc_ofw_header {
  uint32_t ofw_magic;
  uint32_t ofw_version;
  uint32_t cif_handler;
  uint32_t irq_desc_table;
} __attribute__((packed));

struct efi_info {
  uint32_t efi_loader_signature;
  uint32_t efi_systab;
  uint32_t efi_memdesc_size;
  uint32_t efi_memdesc_version;
  uint32_t efi_memmap;
  uint32_t efi_memmap_size;
  uint32_t efi_systab_hi;
  uint32_t efi_memmap_hi;
};

#define E820_MAX_ENTRIES_ZEROPAGE 128

enum {
  E820_USABLE   = 1,
  E820_RESERVED   = 2,
  E820_ACPI_RECLAIM = 3,
  E820_APCI_NVS   = 4,
  E820_UNUSABLE   = 5,
};

struct boot_e820_entry {
  uint64_t addr;
  uint64_t size;
  uint32_t type;
} __attribute__((packed));

#define JAILHOUSE_SETUP_REQUIRED_VERSION  1

struct jailhouse_setup_data {
  struct {
    uint16_t  version;
    uint16_t  compatible_version;
  } __attribute__((packed)) hdr;
  struct {
    uint16_t  pm_timer_address;
    uint16_t  num_cpus;
    uint64_t  pci_mmconfig_base;
    uint32_t  tsc_khz;
    uint32_t  apic_khz;
    uint8_t standard_ioapic;
    uint8_t cpu_ids[255];
  } __attribute__((packed)) v1;
  struct {
    uint32_t  flags;
  } __attribute__((packed)) v2;
} __attribute__((packed));

struct apm_bios_info {
  uint16_t  version;
  uint16_t  cseg;
  uint32_t  offset;
  uint16_t  cseg_16;
  uint16_t  dseg;
  uint16_t  flags;
  uint16_t  cseg_len;
  uint16_t  cseg_16_len;
  uint16_t  dseg_len;
};

struct screen_info {
  uint8_t  orig_x;
  uint8_t  orig_y;
  uint16_t ext_mem_k;
  uint16_t orig_video_page;
  uint8_t  orig_video_mode;
  uint8_t  orig_video_cols;
  uint8_t  flags;
  uint8_t  unused2;
  uint16_t orig_video_ega_bx;
  uint16_t unused3;
  uint8_t  orig_video_lines;
  uint8_t  orig_video_isVGA;
  uint16_t orig_video_points;

  uint16_t lfb_width;
  uint16_t lfb_height;
  uint16_t lfb_depth;
  uint32_t lfb_base;
  uint32_t lfb_size;
  uint16_t cl_magic, cl_offset;
  uint16_t lfb_linelength;
  uint8_t  red_size;
  uint8_t  red_pos;
  uint8_t  green_size;
  uint8_t  green_pos;
  uint8_t  blue_size;
  uint8_t  blue_pos;
  uint8_t  rsvd_size;
  uint8_t  rsvd_pos;
  uint16_t vesapm_seg;
  uint16_t vesapm_off;
  uint16_t pages;
  uint16_t vesa_attributes;
  uint32_t capabilities;
  uint32_t ext_lfb_base;
  uint8_t  _reserved[2];
} __attribute__((packed));

struct ist_info {
  uint32_t signature;
  uint32_t command;
  uint32_t event;
  uint32_t perf_level;
};

struct edid_info {
  unsigned char dummy[128];
};


#define EDD_MBR_SIG_MAX 16
#define EDDMAXNR 6

struct edd_device_params {
  uint16_t length;
  uint16_t info_flags;
  uint32_t num_default_cylinders;
  uint32_t num_default_heads;
  uint32_t sectors_per_track;
  uint64_t number_of_sectors;
  uint16_t bytes_per_sector;
  uint32_t dpte_ptr;
  uint16_t key;
  uint8_t device_path_info_length;
  uint8_t reserved2;
  uint16_t reserved3;
  uint8_t host_bus_type[4];
  uint8_t interface_type[8];
  union {
    struct {
      uint16_t base_address;
      uint16_t reserved1;
      uint32_t reserved2;
    } __attribute__ ((packed)) isa;
    struct {
      uint8_t bus;
      uint8_t slot;
      uint8_t function;
      uint8_t channel;
      uint32_t reserved;
    } __attribute__ ((packed)) pci;
    struct {
      uint64_t reserved;
    } __attribute__ ((packed)) ibnd;
    struct {
      uint64_t reserved;
    } __attribute__ ((packed)) xprs;
    struct {
      uint64_t reserved;
    } __attribute__ ((packed)) htpt;
    struct {
      uint64_t reserved;
    } __attribute__ ((packed)) unknown;
  } interface_path;
  union {
    struct {
      uint8_t device;
      uint8_t reserved1;
      uint16_t reserved2;
      uint32_t reserved3;
      uint64_t reserved4;
    } __attribute__ ((packed)) ata;
    struct {
      uint8_t device;
      uint8_t lun;
      uint8_t reserved1;
      uint8_t reserved2;
      uint32_t reserved3;
      uint64_t reserved4;
    } __attribute__ ((packed)) atapi;
    struct {
      uint16_t id;
      uint64_t lun;
      uint16_t reserved1;
      uint32_t reserved2;
    } __attribute__ ((packed)) scsi;
    struct {
      uint64_t serial_number;
      uint64_t reserved;
    } __attribute__ ((packed)) usb;
    struct {
      uint64_t eui;
      uint64_t reserved;
    } __attribute__ ((packed)) i1394;
    struct {
      uint64_t wwid;
      uint64_t lun;
    } __attribute__ ((packed)) fibre;
    struct {
      uint64_t identity_tag;
      uint64_t reserved;
    } __attribute__ ((packed)) i2o;
    struct {
      uint32_t array_number;
      uint32_t reserved1;
      uint64_t reserved2;
    } __attribute__ ((packed)) raid;
    struct {
      uint8_t device;
      uint8_t reserved1;
      uint16_t reserved2;
      uint32_t reserved3;
      uint64_t reserved4;
    } __attribute__ ((packed)) sata;
    struct {
      uint64_t reserved1;
      uint64_t reserved2;
    } __attribute__ ((packed)) unknown;
  } device_path;
  uint8_t reserved4;
  uint8_t checksum;
} __attribute__ ((packed));

struct edd_info {
  uint8_t device;
  uint8_t version;
  uint16_t interface_support;
  uint16_t legacy_max_cylinder;
  uint8_t legacy_max_head;
  uint8_t legacy_sectors_per_track;
  struct edd_device_params params;
} __attribute__ ((packed));

struct boot_params {
  struct screen_info screen_info;
  struct apm_bios_info apm_bios_info;
  uint8_t  _pad2[4];
  uint64_t  tboot_addr;
  struct ist_info ist_info;
  uint64_t acpi_rsdp_addr;
  uint8_t  _pad3[8];
  uint8_t  hd0_info[16];
  uint8_t  hd1_info[16];
  struct sys_desc_table sys_desc_table;
  struct olpc_ofw_header olpc_ofw_header;
  uint32_t ext_ramdisk_image;
  uint32_t ext_ramdisk_size;
  uint32_t ext_cmd_line_ptr;
  uint8_t  _pad4[116];
  struct edid_info edid_info;
  struct efi_info efi_info;
  uint32_t alt_mem_k;
  uint32_t scratch;
  uint8_t  e820_entries;
  uint8_t  eddbuf_entries;
  uint8_t  edd_mbr_sig_buf_entries;
  uint8_t  kbd_status;
  uint8_t  secure_boot;
  uint8_t  _pad5[2];
  uint8_t  sentinel;
  uint8_t  _pad6[1];
  struct setup_header hdr;
  uint8_t  _pad7[0x290-0x1f1-sizeof(struct setup_header)];
  uint32_t edd_mbr_sig_buffer[EDD_MBR_SIG_MAX];
  struct boot_e820_entry e820_table[E820_MAX_ENTRIES_ZEROPAGE];
  uint8_t  _pad8[48];
  struct edd_info eddbuf[EDDMAXNR];
  uint8_t  _pad9[276];
} __attribute__((packed));

enum x86_hardware_subarch {
  X86_SUBARCH_PC = 0,
  X86_SUBARCH_LGUEST,
  X86_SUBARCH_XEN,
  X86_SUBARCH_INTEL_MID,
  X86_SUBARCH_CE4100,
  X86_NR_SUBARCHS,
};

#endif
