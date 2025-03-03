#pragma once

// Size type
#if __SIZEOF_SIZE_T__ == 4
typedef uint32_t efi_size_t;
typedef int32_t efi_ssize_t;
#elif __SIZEOF_SIZE_T__ == 8
typedef uint64_t efi_size_t;
typedef int64_t efi_ssize_t;
#else
#error Cannot determine size type
#endif

// Fixed width types
typedef uint8_t efi_u8_t;
typedef int8_t efi_i8_t;
typedef uint16_t efi_u16_t;
typedef int16_t efi_i16_t;
typedef uint32_t efi_u32_t;
typedef int32_t efi_i32_t;
typedef uint64_t efi_u64_t;
typedef int64_t efi_i64_t;

#if __SIZEOF_LONG__ == 8
#define __PRI64_PREFIX  L"l"
#elif __SIZEOF_LONG_LONG__ == 8
#define __PRI64_PREFIX  L"ll"
#else
#error Cannot determine 64-bit type
#endif

#define EFI_PRId8   L"d"
#define EFI_PRId16  L"d"
#define EFI_PRId32  L"d"
#define EFI_PRId64  __PRI64_PREFIX L"d"

#define EFI_PRIi8   L"i"
#define EFI_PRIi16  L"i"
#define EFI_PRIi32  L"i"
#define EFI_PRIi64  __PRI64_PREFIX L"i"

#define EFI_PRIo8   L"o"
#define EFI_PRIo16  L"o"
#define EFI_PRIo32  L"o"
#define EFI_PRIo64  __PRI64_PREFIX L"o"

#define EFI_PRIu8   L"u"
#define EFI_PRIu16  L"u"
#define EFI_PRIu32  L"u"
#define EFI_PRIu64  __PRI64_PREFIX L"u"

#define EFI_PRIx8   L"x"
#define EFI_PRIx16  L"x"
#define EFI_PRIx32  L"x"
#define EFI_PRIx64  __PRI64_PREFIX L"x"

#define EFI_PRIX8   L"X"
#define EFI_PRIX16  L"X"
#define EFI_PRIX32  L"X"
#define EFI_PRIX64  __PRI64_PREFIX L"X"

// Max width type
typedef uintmax_t efi_umax_t;
typedef intmax_t efi_imax_t;

#if __UINTMAX_MAX__ != __UINT64_MAX__ || \
    __INTMAX_MAX__ != __INT64_MAX__
#error (u)intmax_t are not 64-bit types
#endif

#define EFI_PRIdMAX __PRI64_PREFIX "d"
#define EFI_PRIiMAX __PRI64_PREFIX "i"
#define EFI_PRIoMAX __PRI64_PREFIX "o"
#define EFI_PRIuMAX __PRI64_PREFIX "u"
#define EFI_PRIxMAX __PRI64_PREFIX "x"
#define EFI_PRIXMAX __PRI64_PREFIX "X"

// Pointer width type
typedef uintptr_t efi_uptr_t;
typedef intptr_t efi_iptr_t;

#if __SIZEOF_POINTER__ == 4
#define __PRIPTR_PREFIX L""
#elif __SIZEOF_POINTER__ == 8
#define __PRIPTR_PREFIX __PRI64_PREFIX
#else
#error Cannot determine pointer size
#endif

#define EFI_PRIdPTR __PRIPTR_PREFIX "d"
#define EFI_PRIiPTR __PRIPTR_PREFIX "i"
#define EFI_PRIoPTR __PRIPTR_PREFIX "o"
#define EFI_PRIuPTR __PRIPTR_PREFIX "u"
#define EFI_PRIxPTR __PRIPTR_PREFIX "x"
#define EFI_PRIXPTR __PRIPTR_PREFIX "X"
