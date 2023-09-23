/*
 * EFI Structure definitions
 */
#ifndef EFI_H
#define EFI_H

/* Freestanding compiler headers */
#include <stddef.h>
#include <stdint.h>

/* CPU specific header */
#include <cpu.h>

// EFI handles
typedef void *efi_handle_t;

// EFI chars
typedef efi_u8_t efi_ch8_t;
typedef efi_u16_t efi_ch16_t;

// EFI bool
typedef efi_u8_t efi_bool_t;
#define true  ((efi_bool_t) 1)
#define false ((efi_bool_t) 0)

// EFI GUID
typedef struct {
  efi_u32_t data1;
  efi_u16_t data2;
  efi_u16_t data3;
  efi_u8_t  data4[8];
} efi_guid_t;

// EFI Inteface type
typedef enum {
  EFI_NATIVE_INTERFACE
} efi_interface_type_t;

// EFI time
typedef struct {
  efi_u16_t year;
  efi_u8_t  month;
  efi_u8_t  day;
  efi_u8_t  hour;
  efi_u8_t  minute;
  efi_u8_t  second;
  efi_u8_t  pad1;
  efi_u32_t nanosecond;
  efi_i16_t time_zone;
  efi_u8_t  daylight;
  efi_u8_t  pad2;
} efi_time_t;

typedef struct {
  efi_u32_t  resolution;
  efi_u32_t  accuracy;
  efi_bool_t sets_to_zero;
} efi_time_cap_t;

// EFI status
#include <efi_status.h>

// Physical address
typedef efi_u64_t efi_physical_address_t;

// Virtual address
typedef efi_u64_t efi_virtual_address_t;

// Task priority type
typedef efi_size_t efi_tpl_t;

#define TPL_APPLICATION 4
#define TPL_CALLBACK    8
#define TPL_NOTIFY      16
#define TPL_HIGH_LEVEL  31

// EFI event type and related definitions
typedef void *efi_event_t;

#define EVT_TIMER                         0x80000000
#define EVT_RUNTIME                       0x40000000
#define EVT_NOTIFY_WAIT                   0x00000100
#define EVT_NOTIFY_SIGNAL                 0x00000200
#define EVT_SIGNAL_EXIT_BOOT_SERVICES     0x00000201
#define EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE 0x60000202

typedef efi_status_t (efiapi *efi_event_t_notify)(efi_event_t event, void *context);

typedef enum {
  EFI_TIMER_CANCEL,
  EFI_TIMER_PERIODIC,
  EFI_TIMER_RELATIVE
} efi_timer_delay_t;

// Allocation type
typedef enum {
  EFI_ALLOCATE_ANY_PAGES,
  EFI_ALLOCATE_MAX_ADDRESS,
  EFI_ALLOCATE_FIXED_ADDRESS,
  EFI_ALLOCATE_TYPE_MAX
} efi_allocate_type_t;

// Memory type
typedef enum {
  EFI_RESERVED_MEMORY_TYPE,
  EFI_LOADER_CODE,
  EFI_LOADER_DATA,
  EFI_BOOT_SERVICES_CODE,
  EFI_BOOT_SERVICES_DATA,
  EFI_RUNTIME_SERVICES_CODE,
  EFI_RUNTIME_SERVICES_DATA,
  EFI_CONVENTIONAL_MEMORY,
  EFI_UNUSABLE_MEMORY,
  EFI_ACPI_RECLAIM_MEMORY,
  EFI_ACPI_MEMORY_NVS,
  EFI_MEMORY_MAPPED_IO,
  EFI_MEMORY_MAPPED_IO_PORT_SPACE,
  EFI_PAL_CODE,
  EFI_MAX_MEMORY_TYPE
} efi_memory_type_t;

// Memory descriptor
typedef struct {
  efi_u32_t               type;
  efi_physical_address_t  start;
  efi_virtual_address_t   virtual_start;
  efi_u64_t               number_of_pages;
  efi_u64_t               attribute;
} efi_memory_descriptor_t;

// Locate search type
typedef enum {
  EFI_LOCATE_ALL_HANDLES,
  EFI_LOCATE_BY_REGISTER_NOTIFY,
  EFI_LOCATE_BY_PROTOCOL
} efi_locate_search_type_t;

// Information entry about a protocol
typedef struct {
  efi_handle_t agent_handle;
  efi_handle_t controller_handle;
  efi_u32_t attrib;
  efi_u32_t open_count;
} efi_open_protocol_information_entry_t;

// Protocol headers depend on the system table
typedef struct efi_system_table efi_system_table_t;

// Protocol headers
#include <protocol/efi_simple_text_out.h>
#include <protocol/efi_simple_text_in.h>
#include <protocol/efi_device_path.h>
#include <protocol/efi_loaded_image.h>
#include <protocol/efi_file.h>
#include <protocol/efi_simple_file_system.h>
#include <protocol/efi_graphics_output.h>
#include <protocol/efi_unicode_collation.h>
#include <protocol/efi_hii_database.h>

// Non-protocol GUIDs
#define EFI_GLOBAL_VARIABLE \
  { 0x8BE4DF61, 0x93CA, 0x11d2, 0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C }

#define EFI_ACPI_TABLE_GUID \
  { 0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81 }

// EFI tables
typedef struct {
  efi_u64_t signature;
  efi_u32_t revision;
  efi_u32_t header_size;
  efi_u32_t crc32;
  efi_u32_t reserved;
} efi_table_header_t;

typedef struct {
  efi_table_header_t hdr;

  // TPL services
  efi_tpl_t (efiapi *raise_tpl)(efi_tpl_t new_tpl);
  void (efiapi *restore_tpl)(efi_tpl_t old_tpl);

  // Memory allocation services
  efi_status_t (efiapi *allocate_pages)(
    efi_allocate_type_t type,
    efi_memory_type_t memory_type,
    efi_size_t pages,
    efi_physical_address_t *memory);
  efi_status_t (efiapi *free_pages)(efi_physical_address_t memory, efi_size_t pages);
  efi_status_t (efiapi *get_memory_map)(
    efi_size_t *memory_map_size,
    efi_memory_descriptor_t *memory_map,
    efi_size_t *map_key,
    efi_size_t *descriptor_size,
    efi_u32_t *descriptor_version);
  efi_status_t (efiapi *allocate_pool)(efi_memory_type_t pool_type, efi_size_t size, void **buffer);
  efi_status_t (efiapi *free_pool)(void *buffer);

  // Event services
  efi_status_t (efiapi *create_event)(
    efi_u32_t type,
    efi_tpl_t notify_tpl,
    efi_event_t_notify notify_function,
    void *notify_context,
    efi_event_t *event);
  efi_status_t (efiapi *set_timer)(efi_event_t event, efi_timer_delay_t type, efi_u64_t trigger_time);
  efi_status_t (efiapi *wait_for_event)(efi_size_t num_events, efi_event_t *event, efi_size_t *index);
  efi_status_t (efiapi *signal_event)(efi_event_t event);
  efi_status_t (efiapi *close_event)(efi_event_t event);
  efi_status_t (efiapi *check_event)(efi_event_t event);

  // Protocol services
  efi_status_t (efiapi *install_protocol_interface)(
    efi_handle_t *handle,
    efi_guid_t *protocol,
    efi_interface_type_t interface_type,
    void *interface);
  efi_status_t (efiapi *reinstall_protocol_interface)(
    efi_handle_t handle,
    efi_guid_t *protocol,
    void *old_interface,
    void *new_interface);
  efi_status_t (efiapi *uninstall_protocol_interface)(efi_handle_t handle, efi_guid_t *protocol, void *interface);
  efi_status_t (efiapi *handle_protocol)(efi_handle_t handle, efi_guid_t *protocol, void **interface);
  void *reserved;
  efi_status_t (efiapi *register_protocol_notify)(efi_guid_t *protocol, efi_event_t event, void **registration);
  efi_status_t (efiapi *locate_handle)(
    efi_locate_search_type_t search_type,
    efi_guid_t *protocol,
    void *search_key,
    efi_size_t *buffer_size,
    efi_handle_t *buffer);
  efi_status_t (efiapi *locate_device_path)(
    efi_guid_t *protocol,
    efi_device_path_protocol_t **device_path,
    efi_handle_t *device);
  efi_status_t (efiapi *install_configuration_table)(efi_guid_t *guid, void *table);

  // Image services
  efi_status_t (efiapi *load_image)(
    efi_bool_t boot_policy,
    efi_handle_t parent_image_handle,
    efi_device_path_protocol_t *device_path,
    void *source_buffer,
    efi_size_t source_size,
    efi_handle_t *image_handle);
  efi_status_t (efiapi *start_image)(efi_handle_t image_handle, efi_size_t *exit_data_size, efi_ch16_t **exit_data);
  efi_status_t (efiapi *exit)(
    efi_handle_t image_handle,
    efi_status_t exit_status,
    efi_size_t exit_data_size,
    efi_ch16_t *exit_data);
  efi_status_t (efiapi *unload_image)(efi_handle_t image_handle);
  efi_status_t (efiapi *exit_boot_services)(efi_handle_t image_handle, efi_size_t map_key);

  // Misc services
  efi_status_t (efiapi *get_next_monotonic_count)(efi_u64_t *count);
  efi_status_t (efiapi *stall)(efi_size_t microseconds);
  efi_status_t (efiapi *set_watchdog_timer)(
    efi_size_t timeout,
    efi_u64_t watchdog_code,
    efi_size_t data_size,
    efi_ch16_t *watchdog_data);

  // Driver support services
  efi_status_t (efiapi *connect_controller)(
    efi_handle_t controller_handle,
    efi_handle_t *driver_image_handle,
    efi_device_path_protocol_t *remaining_device_path,
    efi_bool_t recursive);
  efi_status_t (efiapi *disconnect_controller)(
    efi_handle_t controller_handle,
    efi_handle_t driver_image_handle,
    efi_handle_t child_handle);

  // Open and close protocol services
  efi_status_t (efiapi *open_protocol)(
    efi_handle_t handle,
    efi_guid_t *protocol,
    void **interface,
    efi_handle_t agent_handle,
    efi_handle_t controller_handle,
    efi_u32_t attrib);
  efi_status_t (efiapi *close_protocol)(
    efi_handle_t handle,
    efi_guid_t *protocol,
    efi_handle_t agent_handle,
    efi_handle_t controller_handle);
  efi_status_t (efiapi *open_protocol_information)(
    efi_handle_t handle,
    efi_guid_t *protocol,
    efi_open_protocol_information_entry_t **entry_buffer,
    efi_size_t *entry_count);

  // Library services
  efi_status_t (efiapi *protocols_per_handle)(
    efi_handle_t handle,
    efi_guid_t ***protocol_buffer,
    efi_size_t *protocol_buffer_count);
  efi_status_t (efiapi *locate_handle_buffer)(
    efi_locate_search_type_t search_type,
    efi_guid_t *protocol,
    void *search_key,
    efi_size_t *handle_count,
    efi_handle_t **handle_buffer);
  efi_status_t (efiapi *locate_protocol)(efi_guid_t *protocol, void *registration, void **interface);
  efi_status_t (efiapi *install_multiple_protocol_interfaces)(efi_handle_t *handle, ...);
  efi_status_t (efiapi *uninstall_multiple_protocol_interfaces)(efi_handle_t handle, ...);

  // CRC32
  efi_status_t (efiapi *calculate_crc32)(void *data, efi_size_t data_size, efi_u32_t *crc32);

  // Misc services
  void (efiapi *copy_mem)(void *dest, void *src, efi_size_t length);
  void (efiapi *set_mem)(void *buffer, efi_size_t size, efi_u8_t value);

  // Extended event creation (UEFI 2.0+ only)
  efi_status_t (efiapi *create_event_ex)(
    efi_u32_t type,
    efi_tpl_t notify_tpl,
    efi_event_t_notify notify_function,
    void *notify_context,
    efi_guid_t *event_group,
    efi_event_t *event);
} efi_boot_services_t;

// Types of resets
typedef enum {
  EFI_RESET_COLD,
  EFI_RESET_WARM,
  EFI_RESET_SHUTDOWN,
  EFI_RESET_PLATFORM_SPECIFIC
} efi_reset_type_t;

typedef struct {
  efi_table_header_t hdr;

  // Time services
  efi_status_t (efiapi *get_time)(efi_time_t *time, efi_time_cap_t *cap);
  efi_status_t (efiapi *set_time)(efi_time_t *time);
  efi_status_t (efiapi *get_wakeup_time)(efi_bool_t *enabled, efi_bool_t *pending, efi_time_t *time);
  efi_status_t (efiapi *set_wakeup_time)(efi_bool_t enable, efi_time_t *time);

  // Virtual memory services
  efi_status_t (efiapi *set_virtual_address_map)(
    efi_size_t memory_map_size,
    efi_size_t desc_size,
    efi_size_t desc_version,
    efi_memory_descriptor_t *virtual_map);
  efi_status_t (efiapi *convert_pointer)(efi_size_t debug_disposition, void **address);

  // Variable services
  efi_status_t (efiapi *get_variable)(
    efi_ch16_t *variable_name,
    efi_guid_t *vendor_guid,
    efi_u32_t *attrib,
    efi_size_t *data_size,
    void *data);
  efi_status_t (efiapi *get_next_variable_name)(
    efi_size_t *variable_name_size,
    efi_ch16_t *variable_name,
    efi_guid_t *vendor_guid);
  efi_status_t (efiapi *set_variable)(
    efi_ch16_t *variable_name,
    efi_guid_t *vendor_guid,
    efi_u32_t attrib,
    efi_size_t data_size,
    void *data);

  // Misc services
  efi_status_t (efiapi *get_next_high_monotonic_count)(efi_u32_t *high_count);
  void (efiapi *reset_system)(
    efi_reset_type_t reset_type,
    efi_status_t reset_status,
    efi_size_t data_size,
    void *reset_data);

  // Capsule services (UEFI 2.0+)
  void *update_capsule;
  void *query_capsule_capabilities;

  // Variable infromation service (UEFI 2.0+)
  efi_status_t (efiapi *query_variable_info)(
    efi_u32_t attrib,
    efi_u64_t *max_storage_size,
    efi_u64_t *rem_storage_size,
    efi_u64_t *max_size);
} efi_runtime_services_t;

// EFI configuration table
typedef struct {
  efi_guid_t    vendor_guid;
  void        *vendor_table;
} efi_configuration_table_t;

// EFI system table
struct efi_system_table {
  efi_table_header_t             hdr;
  efi_ch16_t                     *vendor;
  efi_u32_t                      revision;
  efi_handle_t                   con_in_handle;
  efi_simple_text_in_protocol_t  *con_in;
  efi_handle_t                   con_out_handle;
  efi_simple_text_out_protocol_t *con_out;
  efi_handle_t                   std_err_handle;
  efi_simple_text_out_protocol_t *std_err;
  efi_runtime_services_t         *runtime_services;
  efi_boot_services_t            *boot_services;
  efi_size_t                     cnt_config_entries;
  efi_configuration_table_t      *config_entries;
};

#endif
