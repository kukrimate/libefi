//
// Implementation of the Unicode Collation protocol
//

#include <efi.h>
#include "unicode.h"

static efi_unicode_collation_protocol uemu_unicode_collation = {

};

efi_unicode_collation_protocol *unicode_collation(void)
{
    return &uemu_unicode_collation;
}
