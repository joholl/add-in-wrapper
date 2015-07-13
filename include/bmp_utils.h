/*********************************************
**
**  Bitmap reading module header.
**
*********************************************/

#ifndef _BMP_UTILS_H
	#define _BMP_UTILS_H 1

// Type definition.
	#ifndef _UINT8_T
		typedef unsigned char uint8_t;
	#endif // _UINT8_T

// Structure definition.
	struct Bitmap
	{
		unsigned int width, height;
		unsigned int depth;
		const uint8_t *data;
	};

// Function prototypes.
	void bitmap_read(const char *file, unsigned int width, unsigned int height, uint8_t *data);
	void bitmap_pixels(const struct Bitmap *bmp, uint8_t *address);

#endif // BMP_UTILS_H
