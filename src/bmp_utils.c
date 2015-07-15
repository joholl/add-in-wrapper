/*
	Bitmap reading module.
*/



/*
	Header inclusions.
*/

// Standard headers.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Project headers.
#include "error.h"
#include "bmp_utils.h"



/*
	Composed types definitions.

	These types are used only in this file.
*/

// Bitmap meta-information structure definition.
struct Bitmap
{
	unsigned int width, height;
	unsigned int depth;
	const uint8_t *data;
};



/*
	Static declarations.
*/

static int bitmap_pixels(const struct Bitmap *bmp, uint8_t *address);



/*
	bitmap_read()

	Reads a bitmap file and copies its data to the given pointer. The data
	should be only black and white. If not, a warning is emitted and each
	pixel is turned into the closer color according to an arithmetic mean.

	@arg	file	File to read.
	@arg	width	Bitmap width.
	@arg	height	Bitmap height.
	@arg	data	Memory area to copy data to.
*/

void bitmap_read(const char *file, unsigned int width, unsigned int height,
	uint8_t *data_ptr)
{
	/*
		Variables declaration.
	*/

	// Using an integer to read parts of bitmap header.
	uint32_t l;
	// Using a short-named pointer to bitmap data.
	uint8_t *data;
	// Using a bitmap structure.
	struct Bitmap *bmp;
	// Using a long to store the file size to read.
	long size;
	// Using a file pointer.
	FILE *fp;

	// Opening the bitmap file.
	fp = fopen(file,"r");
	// Emitting an error on failure.
	if(!fp)
	{
		// Emitting a bmp-no-open error.
		error_emit(ERROR, "bmp-no-open", file);
		// Also returning from function.
		return;
	}



	/*
		Reading file.
	*/

	// Allocating memory for the size of a bitmap.
	bmp = (struct Bitmap *)malloc(sizeof(struct Bitmap));
	// Emitting an error on alloc failure.
	if(!bmp)
	{
		// Emitting a bmp-alloc error.
		error_emit(ERROR,"bmp-alloc",file);
		// Closing the file pointer.
		fclose(fp);
		// Returning from function.
		return;
	}

	// Getting the file size. Seeking the file end.
	fseek(fp, 0, SEEK_END);
	// Allocating memory for the size of the file.
	data = (uint8_t *)malloc(size = ftell(fp) + 1);
	// Setting structure data pointer.
	bmp->data = data;
	// Moving the file pointer to the beginning of the file.
	fseek(fp, 0, SEEK_SET);
	// Handling alloc failure.
	if(!bmp->data)
	{
		// Emitting a bmp-alloc error.
		error_emit(ERROR,"bmp-alloc",file);
		// Freeing the previous allocated bitmap pointer.
		free(bmp);
		// Closing the file.
		fclose(fp);
		// Finally returning from function.
		return;
	}

	// Reading file data to the allocated memory.
	fread((void *)bmp->data, size, 1, fp);
	// Closing the file.
	fclose(fp);

	// Getting bitmap signature.
	l = (data[0] << 8) | data[1];
	// Checking if the signature is one of BM, BA, CI, CP, IC or PT.
	if(l != 0x424d && l != 0x4241 && l != 4349 && l != 4350 && l != 4943
		&& l!=5054)
	{
		// If not, emitting an error.
		error_emit(ERROR, "bmp-valid", file);
		// Also returning.
		return;
	}

	// Getting image width.
	l = (data[21] << 24) | (data[20] << 16) | (data[19] << 8) | data[18];
	// If it doesn't match the wanted width, emit a warning.
	if(l != width) error_emit(ERROR, "bmp-width", file, l, width);
	// Setting the structure attribute.
	bmp->width = l;

	// Getting image height.
	l = (data[25] << 24) | (data[24] << 16) | (data[23] << 8) | data[22];
	// If it doesn't match the wanted height, emit a warning.
	if(l != height) error_emit(WARNING, "bmp-height", file, l, height);
	// Setting the structure attribute.
	bmp->height = l;

	// Getting the bitmap depth.
	l = (data[29] << 8) | data[28];
	// Emitting an error if it's not supported.
	if(l != 1 && l != 16 && l != 24 && l != 32)
	{
		// Emitting an error.
		error_emit(ERROR,"bmp-depth",file,l);
		// Returning from function.
		return;
	}
	// Emitting a warning for 16-bit bitmaps, that aren't fully supported.
	if(l == 16) error_emit(WARNING, "bmp-16-bit", file);
	// Setting the structure member.
	bmp->depth = l;

	// Reading bitmap information into the given data pointer.
	l = bitmap_pixels(bmp, data_ptr);

	// If the bitmap has non purely-black-and-white pixels, emit a warning.
	if(l) error_emit(WARNING, "bmp-color", file);
}

/*
	bitmap_pixels()

	Extracts the pixels from a bitmap and writes them to a preallocated
	memory area in black-and-white indexed format.

	@arg	bmp	Bitmap structure to read data from.
	@arg	address	Address to write bitmap pixels to.

	@return		1 if non-black-and-white pixels are found, 0 otherwise.
*/

static int bitmap_pixels(const struct Bitmap *bmp, uint8_t *address)
{
	// Using the offset of raw pixel data in the bitmap data.
	const unsigned int offset =
		(bmp->data[0x0d] << 24) | (bmp->data[0x0c] << 16) |
		(bmp->data[0x0b] << 8) | bmp->data[0x0a];
	// Using a direct pointer to pixel data.
	const uint8_t *data = bmp->data + offset;

	// Using an integer to store the line length.
	int line_length = (bmp->depth == 1 ? 4 : 30 * (bmp->depth >> 3));
	// Using a warning indicator, related to the presence of pixels in the
	// image, that are neither black nor white.
	int warning = 0;
	// Using two iterators, an offset and a dummy integer.
	int x, y, v, s;
	// Using a byte.
	uint8_t byte;

	// Adapting line length because it needs to be divisible by 4.
	line_length += line_length & 3;

	// Emptying the existing bitmap.
	for(x=0; x < 76; x++) address[x] = 0;

	for(y=18; y+1; y--) for(x=0; x<30; x++)
	{
		// Extracting a pixel using various methods depending on the
		// bitmap depth.
		switch(bmp->depth)
		{
		// 32-bit assumes X8-R8-G8-B8 or A8-R8-G8-B8.
		case 32:
			// Computing the pixel byte offset.
			v = line_length * y + (x<<2) + 1;
			// Computing the sum of the three channel intensities.
			s = data[v] + data[v+1] + data[v+2];
			// Checking pure-black-and-white warning.
			if(s && s != 765) warning = 1;
			// Reducing to boolean.
			s = (s < 384);
			break;

		// Handling classical 24 bits bitmaps.
		case 24:
			// Computing the pixel byte offset.
			v = line_length * y + x * 3;
			// Extracting the three bytes and getting their sum.
			s = data[v] + data[v+1] + data[v+2];
			// Checking if the pixel is different that strictly
			// black or white.
			if(s && s != 765) warning = 1;
			// Reducing to boolean.
			s = (s < 384);
			break;

		// 16 bits are not well supported (weird decoded values).
		case 16:
			// Extracting two bytes.
			v = line_length * y + (x << 1);
			v = data[v] + data[v+1];
			// Extracting the three sections.
			s  = (v & 0x7c00) >> 12;
			s += (v & 0x03e0) >> 5;
			s += (v & 0x001f);
			// Reducing to boolean.
			s = (s < 23);
			break;

		// Handling monochrome bitmaps.
		case 1: 
			// Extracting the right byte.
			v = line_length * y + (x >> 3);
			// Isolating the sought bit.
			s = data[v] & (128 >> (x & 7));
			break;
		}

		// Getting the offset of the byte to edit.
		v = ((18-y) << 2) + (x >> 3);
		// Getting the byte at this offset.
		byte = address[v];
		// Altering the byte depending on the pixel value.
		if(s) byte |= (128 >> (x & 7));
		else byte &= ~(128 >> (x & 7));
		// Writing the new byte.
		address[v] = byte;
	}

	// Returning the warning indicator.
	return warning;
}

/*
	bitmap_output()

	Outputs a saved bitmap from raw data and size.

	@arg	data	Raw bitmap data, in monochrome format.
	@arg	width	Bitmap width.
	@arg	height	Bitmap height.
	@arg	stream	Stream to output to.
*/

void bitmap_output(uint8_t *data_ptr, int width, int height, FILE *stream)
{
	// Using additional data for the first and last lines.
	uint8_t additional[] = {	
		0x00, 0x00, 0x00, 0x04,
		0x7f, 0xff, 0xff, 0xfc
	};
	// Using a volatile data pointer and a byte.
	uint8_t *data = additional, byte;
	// Using integer coordinates.
	int x, y;
	// Using an iterator offset in the byte.
	int offset;
	// Using a character.
	char c;

	// Iterating over the lines.
	for(y = 0; y < height; y++)
	{
		// Switching to the right data pointer.
		if(y == 1) data = data_ptr;
		else if(y == height - 1) data = additional + 4;

		// Getting next byte.
		byte = *data++;
		// Initializing the byte offset.
		offset = 0;

		// Iterating over the pixels.
		for(x = 0; x < width; x++)
		{

			// Getting new byte if needed.
			if(offset == 8)
			{
				// Getting a new byte.
				byte = *data++;
				// Resetting bit offset.
				offset = 0;
			}

			// Getting the next bit as character.
			c = (byte & 128 ? '#' : ' ');
			// Shifting the current byte.
			byte <<= 1;
			// Updating bit offset.
			offset++;

			// Outputting c twice (better ratio).
			fputc(c, stream);
			fputc(c, stream);
		}

		// Adding a line break.
		fputc('\n', stream);
	}
}
