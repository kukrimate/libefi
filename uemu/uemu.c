/*
 * User mode UEFI emulator
 * Author: Mate Kukri
 */

#ifndef __amd64__
#error The user mode emulator only supports AMD64
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <efi.h>
#include "pe32.h"

// EFI image entry point
typedef efi_status (efiapi *efi_image_entry)(efi_handle, efi_system_table *);

// Error codes
enum {
    SUCCESS     =  0, // Success
    ERR_DISK    = -1, // Disk read error
    ERR_TRUNC   = -2, // Truncated header
    ERR_INVALID = -3, // Invalid header
};

// Find the NT header offset
static int find_filehdr(int fd, u32 *nthdr_offs)
{
    IMAGE_DOS_HEADER doshdr;
    ssize_t len;

    len = pread(fd, &doshdr, sizeof doshdr, 0);
    if (len < 0)                               // Disk read error
        return ERR_DISK;
    if (len < sizeof doshdr)                   // Avoid parsing truncated header
        return ERR_TRUNC;
    if (doshdr.e_magic != IMAGE_DOS_SIGNATURE) // Verify MZ magic
        return ERR_INVALID;

    *nthdr_offs = doshdr.e_lfanew;
    return 0;
}

// Load and validate IMAGE_NT_HEADERS64
static int load_nthdrs64(int fd, u32 nthdr_offs, IMAGE_NT_HEADERS64 *nthdrs64)
{
    ssize_t len;

    len = pread(fd, nthdrs64, sizeof *nthdrs64, nthdr_offs);
    if (len < 0)                                   // Disk read error
        return ERR_DISK;
    if (len < sizeof *nthdrs64)                    // Truncated NT headers
        return ERR_TRUNC;
    if (nthdrs64->Signature != IMAGE_NT_SIGNATURE) // Invalid PE signature
        return ERR_INVALID;
    if (nthdrs64->OptionalHeader.Magic
            != IMAGE_NT_OPTIONAL_HDR64_MAGIC)      // Invalid PE32+ magic
        return ERR_INVALID;
    return 0;
}

// Round val up to the nearest multiple of bound
#define ROUND_UP(val, bound) (((val) + (bound) - 1) / (bound) * (bound))

// Calculate the expected size of all headers
static u32 sizeof_headers_aligned(u32 nthdr_offs, IMAGE_NT_HEADERS64 *nthdrs64)
{
    u32 unaligned_size =
           sizeof nthdrs64->Signature +
           sizeof nthdrs64->FileHeader +
           nthdrs64->FileHeader.SizeOfOptionalHeader +
           nthdrs64->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
    return ROUND_UP(nthdr_offs + unaligned_size, nthdrs64->OptionalHeader.FileAlignment);
}

// Sanity check NT headers
static _Bool validate_nthdrs(u32 nthdrs_offs, IMAGE_NT_HEADERS64 *nthdrs64)
{
    // Verify architecture and subsystem
    if (nthdrs64->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        fputs("Only AMD64 images are supported!\n", stderr);
        return 0;
    }
    if (nthdrs64->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_EFI_APPLICATION) {
        fputs("Invalid subsystem, only EFI Applications are supported!\n", stderr);
        return 0;
    }
    // Make sure the section alingment is a multiple of the page size
    u32 section_align = nthdrs64->OptionalHeader.SectionAlignment;
    if (section_align < getpagesize() || section_align % getpagesize()) {
        fputs("Invalid section alingment, "
                "must be a multiple of the current page size!\n", stderr);
        return 0;
    }
    u32 image_size = nthdrs64->OptionalHeader.SizeOfImage;
    if (image_size % section_align) {
        fputs("Invalid image size, "
                "must be a multiple of the section alingment!\n", stderr);
        return 0;
    }
    u32 header_size = nthdrs64->OptionalHeader.SizeOfHeaders;
    if (header_size != sizeof_headers_aligned(nthdrs_offs, nthdrs64)) {
        fputs("Invalid total headers size, "
                "differs from expected value!\n", stderr);
        return 0;
    }
    if (header_size > image_size) {
        fputs("Invalid total headers size, "
                "can't be greater than total image size!\n", stderr);
        return 0;
    }
    return 1;
}

// Mmap size bytes of aligned memory
// NOTE: align must be a multiple of page size
static void *mmap_aligned(void *preferred, u32 align, u32 size)
{
    // Map memory
    void *mem = mmap(preferred,
                     align + size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (mem == MAP_FAILED)
        return mem;

    // Align if necessary
    uintptr_t align_size = (uintptr_t) mem % align;

    if (align_size) {
        munmap(mem, align_size);
        mem += align_size;
    }

    return mem;
}

// Load PE32 sections based
static int load_pe_sections(int fd, void *image)
{
    printf("Loading PE sections...\n");

    // NT header is at the specified offset in the DOS header
    IMAGE_NT_HEADERS64 *nthdrs64 =
        image + ((IMAGE_DOS_HEADER *) image)->e_lfanew;

    // Section headers are right after the optional headers
    IMAGE_SECTION_HEADER *section =
        (void *) &nthdrs64->OptionalHeader +
            nthdrs64->FileHeader.SizeOfOptionalHeader;

    // Boundaries for writing data
    void *image_cur = image + nthdrs64->OptionalHeader.SizeOfHeaders;
    void *image_end = image + nthdrs64->OptionalHeader.SizeOfImage;

    // Save page size for loop
    int page_size = getpagesize();

    u16 section_cnt = nthdrs64->FileHeader.NumberOfSections;
    for (; section_cnt; --section_cnt, ++section) {

        printf("%.8s\n", section->Name);
        printf("\tCharacteristics: %08x\n", section->Characteristics);
        printf("\tSizeOfRawData:   %08x\n", section->SizeOfRawData);
        printf("\tVirtualSize:     %08x\n", section->Misc.VirtualSize);
        printf("\tVirtualAddress:  %08x\n", section->VirtualAddress);

        void *section_start = image + section->VirtualAddress;
        void *section_end = section_start
            + ROUND_UP(section->Misc.VirtualSize, page_size);

        // Make sure SizeOfRawData fits into the rounded up section size
        if (section->SizeOfRawData > section_end - section_start) {
            return ERR_INVALID;
        }

        // Make sure sections are in sorted order low-to-high addresses and
        // there are no overlapping sections
        if (section_start < image_cur || section_end > image_end) {
            return ERR_INVALID;
        }

        // Zero section
        memset(section_start, 0, section_end - section_start);

        // Read raw section data from file
        ssize_t len = pread(fd, section_start,
                            section->SizeOfRawData,
                            section->PointerToRawData);
        if (len < 0) {
            return ERR_DISK;
        }
        if (len < section->SizeOfRawData) {
            return ERR_TRUNC;
        }


        // Lowest allowed load address after loading this section is the
        // end of said section
        image_cur = section_end;
    }

    return 0;
}

// Apply base relocations
static int apply_pe_relocs(u64 orig_base, void *image)
{
    IMAGE_NT_HEADERS64 *nthdrs = image + ((IMAGE_DOS_HEADER *) image)->e_lfanew;

    // Data directory entry for the base relocations
    IMAGE_DATA_DIRECTORY *reloc_dir =
        nthdrs->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_BASERELOC;

    u32 total_size = reloc_dir->Size;
    IMAGE_BASE_RELOCATION *block = image + reloc_dir->VirtualAddress;

    while (total_size > sizeof *block) {
        u32 block_size = block->SizeOfBlock;
        if (total_size < block_size)
            return ERR_INVALID;

        // Apply block
        printf("Applying reloc block for RVA: %08x\n", block->VirtualAddress);
        u32 reloc_cnt = (block_size - sizeof *block) / sizeof(u16);
        for (u32 i = 0; i < reloc_cnt; ++i) {
            printf("Type: %02d  Offset: %08x\n",
                block->Fixups[i].Type, block->Fixups[i].Offset);
            switch (block->Fixups[i].Type) {
            case IMAGE_REL_BASED_ABSOLUTE: // Do nothing
                break;
            case IMAGE_REL_BASED_DIR64:    // Difference to 64-bit field
                *(u64 *) (image + block->VirtualAddress + block->Fixups[i].Offset)
                             += (u64) image - orig_base;
                break;
            default:                       // Unsupported type
                return ERR_INVALID;
            }
        }

        // Move to next block
        block = (void *) block + block_size;
        total_size -= block_size;
    }

    return 0;
}

// EFI API emulator
extern efi_system_table uemu_st;
void uemu_expose_protocol(efi_guid *with_guid, void *interface);

// Graphics output emulator
efi_graphics_output_protocol *gopemu_init(void);
void gopemu_deinit(efi_graphics_output_protocol *self);

// Early exit mechasnism
static jmp_buf uemu_exit;

static void sigint_handler(int sig)
{
    longjmp(uemu_exit, 1);
}

int main(int argc, char *argv[])
{
    int err = 1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s PEFILE\n", argv[0]);
        goto out;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror(argv[1]);
        goto out;
    }

    // Load and validate image headers
    u32 nthdrs_offs;
    if (find_filehdr(fd, &nthdrs_offs) < 0) {
        fputs("Invalid DOS header!\n", stderr);
        goto out_close;
    }
    IMAGE_NT_HEADERS64 nthdrs64;
    if (load_nthdrs64(fd, nthdrs_offs, &nthdrs64) < 0) {
        fputs("Invalid PE32+ header!\n", stderr);
        goto out_close;
    }
    if (!validate_nthdrs(nthdrs_offs, &nthdrs64)) {
        goto out_close;
    }

    // Map memory for image
    u64 orig_base = nthdrs64.OptionalHeader.ImageBase;
    void *image_addr = mmap_aligned(NULL/*(void *) orig_base*/,
                               nthdrs64.OptionalHeader.SectionAlignment,
                               nthdrs64.OptionalHeader.SizeOfImage);

    if (image_addr == MAP_FAILED) {
        perror("Failed to map memory for image");
        goto out_close;
    }

    printf("Original image base: %p\n", (void *) orig_base);
    printf("Real image base:     %p\n", image_addr);

    // Load headers
    ssize_t len = pread(fd,
                        image_addr,
                        nthdrs64.OptionalHeader.SizeOfHeaders,
                        0);
    if (len < 0) {
        perror("Failed to read headers");
        goto out_close;
    }
    // Load sections
    if (load_pe_sections(fd, image_addr) < 0) {
        fputs("Failed to load section!\n", stderr);
        goto out_close;
    }
    // Perform base relocations if necessary
    if (orig_base != (u64) image_addr) {
        puts("Performing base relocations...");
        if (apply_pe_relocs(orig_base, image_addr) < 0) {
            fputs("Failed to apply base relocations!\n", stderr);
            goto out_close;
        }
    }

    // Catch SIGINT
    signal(SIGINT, sigint_handler);

    // Initialize graphics output emulator
    efi_graphics_output_protocol *gop = gopemu_init();
    uemu_expose_protocol(&(efi_guid) EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, gop);

    if (setjmp(uemu_exit)) {
        puts("Exiting early...");
        goto setjmp_exit;
    }

    puts("\nOutput below from image:\n========================");
    // Call image entry point
    efi_image_entry entry_point =
        image_addr + nthdrs64.OptionalHeader.AddressOfEntryPoint;

    efi_ssize exit_code = entry_point(NULL, &uemu_st);
    if (EFI_ERROR(exit_code)) {
        exit_code &= ~SIZE_MAX_BIT;
        exit_code *= -1;
    }

    printf("========================\n");
    printf("Image exited with code: %ld\n", exit_code);

setjmp_exit:
    // Shutdown GOP emulator
    gopemu_deinit(gop);

    err = 0;
out_close:
    close(fd);
out:
    return err;
}
