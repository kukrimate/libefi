/*
 * EFI simple text output protocol
 */
#ifndef EFI_SIMPLE_TEXT_OUT_H
#define EFI_SIMPLE_TEXT_OUT_H

// EFI boxdraw constants
#define EFI_BOXDRAW_HORIZONTAL                  0x2500
#define EFI_BOXDRAW_VERTICAL                    0x2502
#define EFI_BOXDRAW_DOWN_RIGHT                  0x250c
#define EFI_BOXDRAW_DOWN_LEFT                   0x2510
#define EFI_BOXDRAW_UP_RIGHT                    0x2514
#define EFI_BOXDRAW_UP_LEFT                     0x2518
#define EFI_BOXDRAW_VERTICAL_RIGHT              0x251c
#define EFI_BOXDRAW_VERTICAL_LEFT               0x2524
#define EFI_BOXDRAW_DOWN_HORIZONTAL             0x252c
#define EFI_BOXDRAW_UP_HORIZONTAL               0x2534
#define EFI_BOXDRAW_VERTICAL_HORIZONTAL         0x253c
#define EFI_BOXDRAW_DOUBLE_HORIZONTAL           0x2550
#define EFI_BOXDRAW_DOUBLE_VERTICAL             0x2551
#define EFI_BOXDRAW_DOWN_RIGHT_DOUBLE           0x2552
#define EFI_BOXDRAW_DOWN_DOUBLE_RIGHT           0x2553
#define EFI_BOXDRAW_DOUBLE_DOWN_RIGHT           0x2554
#define EFI_BOXDRAW_DOWN_LEFT_DOUBLE            0x2555
#define EFI_BOXDRAW_DOWN_DOUBLE_LEFT            0x2556
#define EFI_BOXDRAW_DOUBLE_DOWN_LEFT            0x2557
#define EFI_BOXDRAW_UP_RIGHT_DOUBLE             0x2558
#define EFI_BOXDRAW_UP_DOUBLE_RIGHT             0x2559
#define EFI_BOXDRAW_DOUBLE_UP_RIGHT             0x255a
#define EFI_BOXDRAW_UP_LEFT_DOUBLE              0x255b
#define EFI_BOXDRAW_UP_DOUBLE_LEFT              0x255c
#define EFI_BOXDRAW_DOUBLE_UP_LEFT              0x255d
#define EFI_BOXDRAW_VERTICAL_RIGHT_DOUBLE       0x255e
#define EFI_BOXDRAW_VERTICAL_DOUBLE_RIGHT       0x255f
#define EFI_BOXDRAW_DOUBLE_VERTICAL_RIGHT       0x2560
#define EFI_BOXDRAW_VERTICAL_LEFT_DOUBLE        0x2561
#define EFI_BOXDRAW_VERTICAL_DOUBLE_LEFT        0x2562
#define EFI_BOXDRAW_DOUBLE_VERTICAL_LEFT        0x2563
#define EFI_BOXDRAW_DOWN_HORIZONTAL_DOUBLE      0x2564
#define EFI_BOXDRAW_DOWN_DOUBLE_HORIZONTAL      0x2565
#define EFI_BOXDRAW_DOUBLE_DOWN_HORIZONTAL      0x2566
#define EFI_BOXDRAW_UP_HORIZONTAL_DOUBLE        0x2567
#define EFI_BOXDRAW_UP_DOUBLE_HORIZONTAL        0x2568
#define EFI_BOXDRAW_DOUBLE_UP_HORIZONTAL        0x2569
#define EFI_BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE  0x256a
#define EFI_BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL  0x256b
#define EFI_BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL  0x256c

/* EFI block element constants */
#define EFI_BLOCKELEMENT_FULL_BLOCK             0x2588
#define EFI_BLOCKELEMENT_LIGHT_SHADE            0x2591

/* EFI geometric shape constants */
#define EFI_GEOMETRICSHAPE_UP_TRIANGLE          0x25b2
#define EFI_GEOMETRICSHAPE_RIGHT_TRIANGLE       0x25ba
#define EFI_GEOMETRICSHAPE_DOWN_TRIANGLE        0x25bc
#define EFI_GEOMETRICSHAPE_LEFT_TRIANGLE        0x25c4

/* EFI arrow shape constants */
#define EFI_ARROW_LEFT                          0x2190
#define EFI_ARROW_UP                            0x2191
#define EFI_ARROW_RIGHT                         0x2192
#define EFI_ARROW_DOWN                          0x2193

// EFI text colors
#define EFI_BLACK                               0x00
#define EFI_BLUE                                0x01
#define EFI_GREEN                               0x02
#define EFI_CYAN                                0x03
#define EFI_RED                                 0x04
#define EFI_MAGENTA                             0x05
#define EFI_BROWN                               0x06
#define EFI_LIGHTGRAY                           0x07
#define EFI_BRIGHT                              0x08
#define EFI_DARKGRAY                            0x08
#define EFI_LIGHTBLUE                           0x09
#define EFI_LIGHTGREEN                          0x0a
#define EFI_LIGHTCYAN                           0x0b
#define EFI_LIGHTRED                            0x0c
#define EFI_LIGHTMAGENTA                        0x0d
#define EFI_YELLOW                              0x0e
#define EFI_WHITE                               0x0f

/* EFI background colors */
#define EFI_BACKGROUND_BLACK                    0x00
#define EFI_BACKGROUND_BLUE                     0x10
#define EFI_BACKGROUND_GREEN                    0x20
#define EFI_BACKGROUND_CYAN                     0x30
#define EFI_BACKGROUND_RED                      0x40
#define EFI_BACKGROUND_MAGENTA                  0x50
#define EFI_BACKGROUND_BROWN                    0x60
#define EFI_BACKGROUND_LIGHTGRAY                0x70

/* Macro for creating EFI text attributes from colors */
#define EFI_TEXT_ATTR(fg, bg)                   ((fg) | ((bg) << 4))

typedef struct {
  efi_i32_t   max_mode;
  efi_i32_t   mode;
  efi_i32_t   attr;
  efi_i32_t   cursor_col;
  efi_i32_t   cursor_row;
  efi_bool_t  cursor_visible;
} efi_simple_text_out_mode_t;

typedef struct efi_simple_text_out_protocol efi_simple_text_out_protocol_t;

struct efi_simple_text_out_protocol {
  efi_status_t (efiapi *reset) (efi_simple_text_out_protocol_t *self,
    efi_bool_t ext_verf);
  efi_status_t (efiapi *output_string) (efi_simple_text_out_protocol_t *self,
    efi_ch16_t *str);
  efi_status_t (efiapi *test_string) (efi_simple_text_out_protocol_t *self,
    efi_ch16_t *str);
  efi_status_t (efiapi *query_mode) (efi_simple_text_out_protocol_t *self,
    efi_size_t mode_num, efi_size_t* cols, efi_size_t* rows);
  efi_status_t (efiapi *set_mode) (efi_simple_text_out_protocol_t *self,
    efi_size_t mode_num);
  efi_status_t (efiapi *set_attr) (efi_simple_text_out_protocol_t *self,
    efi_size_t attr);
  efi_status_t (efiapi *clear_screen) (efi_simple_text_out_protocol_t *self);
  efi_status_t (efiapi *set_cursor_pos) (efi_simple_text_out_protocol_t *self,
    efi_size_t col, efi_size_t row);
  efi_status_t (efiapi *enable_cursor) (efi_simple_text_out_protocol_t *self,
    efi_bool_t visible);
  efi_simple_text_out_mode_t *mode;
};

#endif
