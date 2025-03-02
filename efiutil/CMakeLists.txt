add_library(efiutil efiutil.c print.c string.c)
target_compile_options(efiutil PRIVATE "-DUSE_EFI110")
target_include_directories(efiutil PUBLIC include)
target_link_libraries(efiutil PUBLIC efiapi)
