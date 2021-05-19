# libefi

This project provides headers to access EFI APIs and a small library called
`efiutil` for formatted printing and a few other conveniences.
EFI images are produced using a proper, PE32(+) targeted MinGW toolchain instead
of the horrible hacks of GNU-EFI.

On Debian the following packages provide appropriate toolchains:

- for IA-32: `binutils-mingw-w64-i686`, `gcc-mingw-w64-i686`
- for AMD64: `binutils-mingw-w64-x86-64`, `gcc-mingw-w64-x86-64`

This repository contains a few example applications for testing/demonstration:

- `hello`  : as simple as it gets, the classic toolchain example
- `bitfont`: bitmap font rendering using UEFI's GOP (Graphics Output Protocol)
- `loadlin`: bootloader for Linux on AMD64 using the old Linux boot protocol

In addition I used this devkit in a few other projects:

- <a href="https://github.com/kukrimate/yaub/">yaub</a>:
    Very small and simple boot manager
- <a href="https://github.com/kukrimate/IntelSpiInfo/">IntelSpiInfo</a>:
    Prints the SPI controller configuration on Intel 6/7 series PCHs
- <a href="https://github.com/kukrimate/grr/">grr</a>:
    Type-1 AMD SVM hypervisor
