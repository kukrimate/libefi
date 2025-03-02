add_executable(hello.efi hello.c)
target_link_options(hello.efi PRIVATE ${LINK_EFI_APPLICATION})
target_link_libraries(hello.efi PRIVATE efiapi efiutil)
