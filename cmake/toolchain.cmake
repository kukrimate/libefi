# Make sure no platform specific assumptions are made
set(CMAKE_SYSTEM_NAME Generic)

# Don't waste time trying to detect the compiler
set(CMAKE_C_COMPILER_FORCED TRUE)

# Set arch specific options
if (${ARCH} STREQUAL amd64)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(ENTRY_POINT_NAME efi_main)
elseif (${ARCH} STREQUAL ia32)
set(CMAKE_SYSTEM_PROCESSOR X86)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(ENTRY_POINT_NAME _efi_main)
else()
message(FATAL_ERROR "Unsupported architecture ${ARCH}")
endif()

# Setup compile flags
add_compile_options(-std=gnu11 -Wall -Wextra -ffreestanding -mgeneral-regs-only)
add_link_options(-nostdlib -Wl,-entry,${ENTRY_POINT_NAME})
link_libraries(-lgcc)

# Provide link options for various types
set(LINK_EFI_APPLICATION -Wl,-subsystem,10)
set(LINK_EFI_BOOT_SERVICES_DRIVER -Wl,-subsystem,11)
set(LINK_EFI_RUNTIME_SERVICES_DRIVER -Wl,-subsystem,12)
