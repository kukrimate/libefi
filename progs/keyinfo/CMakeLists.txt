add_executable(keyinfo.efi keyinfo.c)
target_link_options(keyinfo.efi PRIVATE ${LINK_EFI_APPLICATION})
target_link_libraries(keyinfo.efi PRIVATE efiapi efiutil)
