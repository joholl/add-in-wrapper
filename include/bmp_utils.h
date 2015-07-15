#ifndef _BMP_UTILS_H
	#define _BMP_UTILS_H 1

/*
	Header inclusions.
*/

#include <stdio.h>
#include <stdint.h>



/*
	Function prototypes.
*/

// Load a bitmap from a file, with predefined size.
void bitmap_read(const char *file, unsigned int width, unsigned int height,
	uint8_t *data);
// Output raw bitmap data to stream.
void bitmap_output(uint8_t *data, int width, int height, FILE *stream);

#endif // BMP_UTILS_H
