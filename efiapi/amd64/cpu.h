/*
 * Type definitions for AMD64
 */

#ifndef AMD64_CPU_H
#define AMD64_CPU_H

/* Fixed width types */
typedef uint8_t efi_u8_t;
typedef int8_t efi_i8_t;
typedef uint16_t efi_u16_t;
typedef int16_t efi_i16_t;
typedef uint32_t efi_u32_t;
typedef int32_t efi_i32_t;
typedef uint64_t efi_u64_t;
typedef int64_t efi_i64_t;

/* Size types */
typedef uint64_t efi_size_t;
typedef int64_t efi_ssize_t;

/*
 * Max bit for the size type
 */
#define EFI_SIZE_MAX_BIT 0x8000000000000000

/* Force the Microsoft AMD64 ABI if it's not the default */
#if defined __GNUC__ && !defined _WIN32
#define efiapi __attribute__((ms_abi))
#else
#define efiapi
#endif

#endif
