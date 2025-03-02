add_executable(bitfont.efi bitfont.c fbterm.c)
target_link_options(bitfont.efi PRIVATE ${LINK_EFI_APPLICATION})
target_link_libraries(bitfont.efi PRIVATE efiapi efiutil)
