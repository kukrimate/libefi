/*
 * EFI CPU specific definitions for IA-32
 */

#ifndef __IA32_CPU_H
#define __IA32_CPU_H

/* Fixed width types */
#include <stdint.h>

/* Native width type */
typedef int32_t intn_t;
typedef uint32_t uintn_t;

/* Size type */
typedef uint32_t size_t;

/* Native max bit */
#define MAX_BIT 0x80000000

/* EFI function calling convention */
#define efi_func

#endif
