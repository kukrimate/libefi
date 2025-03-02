add_executable(dumpvar.efi dumpvar.c)
target_link_options(dumpvar.efi PRIVATE ${LINK_EFI_APPLICATION})
target_link_libraries(dumpvar.efi PRIVATE efiapi efiutil)
