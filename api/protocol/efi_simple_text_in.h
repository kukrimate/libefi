//
// EFI simple text input protocol
//
#ifndef EFI_SIMPLE_TEXT_IN_H
#define EFI_SIMPLE_TEXT_IN_H

//
// Unicode control characters
//
#define EFI_CHAR_NUL              0x0000
#define EFI_CHAR_BACKSPACE        0x0008
#define EFI_CHAR_TAB              0x0009
#define EFI_CHAR_LINEFEED         0x000A
#define EFI_CHAR_CARRIAGE_RETURN  0x000D

//
// EFI scan codes
//
#define EFI_SCAN_NULL       0x0000
#define EFI_SCAN_UP         0x0001
#define EFI_SCAN_DOWN       0x0002
#define EFI_SCAN_RIGHT      0x0003
#define EFI_SCAN_LEFT       0x0004
#define EFI_SCAN_HOME       0x0005
#define EFI_SCAN_END        0x0006
#define EFI_SCAN_INSERT     0x0007
#define EFI_SCAN_DELETE     0x0008
#define EFI_SCAN_PAGE_UP    0x0009
#define EFI_SCAN_PAGE_DOWN  0x000A
#define EFI_SCAN_F1         0x000B
#define EFI_SCAN_F2         0x000C
#define EFI_SCAN_F3         0x000D
#define EFI_SCAN_F4         0x000E
#define EFI_SCAN_F5         0x000F
#define EFI_SCAN_F6         0x0010
#define EFI_SCAN_F7         0x0011
#define EFI_SCAN_F8         0x0012
#define EFI_SCAN_F9         0x0013
#define EFI_SCAN_F10        0x0014
#define EFI_SCAN_ESC        0x0017

//
// These are technically not supported by the UEFI spec for this protocol, but
// several existing implementations do
//
#define EFI_SCAN_F11        0x0015
#define EFI_SCAN_F12        0x0016

typedef struct {
  efi_u16_t     scan;
  efi_ch16_t    c;
} efi_in_key_t;

typedef struct efi_simple_text_in_protocol efi_simple_text_in_protocol_t;

struct efi_simple_text_in_protocol {
  efi_status_t (efiapi *reset) (efi_simple_text_in_protocol_t *self,
    efi_bool_t ext_verf);
  efi_status_t (efiapi *read_key) (efi_simple_text_in_protocol_t *self,
    efi_in_key_t *key);
  efi_event_t   wait_for_key;
};

#endif
