//
// PE32+ image loader
//

#ifndef PELOADER_H
#define PELOADER_H

//
// Load a PE32+ image into memory
//
void *load_pe_image(int fd);

//
// Get the entry point of a PE32+ image in memory
//
void *get_pe_entry(void *image);

#endif
