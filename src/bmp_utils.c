/*********************************************
**
**  Bitmap reading module.
**
*********************************************/

#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "bmp_utils.h"

void bitmap_read(const char *file, unsigned int width, unsigned int height, uint8_t *data)
{
	/*
	**  This function read a bitmap and copies its data to the given pointer.
	**  The data should be only black and white. If not, a warning is emitted
	**  and each pixel is converted as the closer color according to an
	**  arithmetic mean.
	*/

	// Declaring variables.
	unsigned int l,i,j;
	struct Bitmap *bmp;
	long size;
	FILE *f;

	// Opening bitmap file.
	f = fopen(file,"r");
	if(!f) { error_emit(ERROR,"bmp-no-open",file); return ; }

	// Allocating memory.
	if(!(bmp = (struct Bitmap *)malloc(sizeof(struct Bitmap))))
		{ error_emit(ERROR,"bmp-alloc",file); return; }
	fseek(f,0,SEEK_END);
	bmp->data = (uint8_t *)malloc(size = ftell(f)+1);
	fseek(f,0,SEEK_SET);
	if(!bmp->data) { fclose(f); error_emit(ERROR,"bmp-alloc",file); return; }

	// Reading file.
	fread((void *)bmp->data,size,1,f);
	fclose(f);

	// Checking bitmap signature.
	l = (bmp->data[0]<<8) + bmp->data[1];
	if(l!=0x424D && l!=0x4241 && l!=4349 && l!=4350 && l!=4943 && l!=5054) {
		error_emit(ERROR,"bmp->data-valid",file); return; }

	// Checking width.
	l = (bmp->data[21]<<24)+(bmp->data[20]<<16)+(bmp->data[19]<<8)+bmp->data[18];
	if(l!=width) error_emit(WARNING,"bmp->data-width",file,l,width);
	bmp->width = l;

	// Checking height.
	l = (bmp->data[25]<<24)+(bmp->data[24]<<16)+(bmp->data[23]<<8)+bmp->data[22];
	if(l!=height) error_emit(WARNING,"bmp->data-height",file,l,height);
	bmp->height = l;

	// Checking depth.
	l = (bmp->data[29]<<8) + bmp->data[28];
	if(l!=1 && l!=16 && l!=24 && l!=32) {
		error_emit(ERROR,"bmp-depth",file,l); return; }
	bmp->depth = l;

	// Reading bitmap information.
	bitmap_pixels(bmp,data);
}

void bitmap_pixels(const struct Bitmap *bmp, uint8_t *address)
{
	unsigned int offset = (bmp->data[0x0D]<<24) | (bmp->data[0x0C]<<16) | (bmp->data[0x0B]<<8) | (bmp->data[0x0A]);
	const uint8_t *data = bmp->data + offset;
	int lineoffset = (bmp->depth==1 ? 4 : 30*bmp->depth>>3);
	int warning = 0;
	int x,y,s;

	lineoffset += lineoffset & 3;
	for(x=0;x<76;x++) address[x] = 0;

	for(y=18;y+1;y--/*,fputc('\n',stdout)*/) for(x=0;x<30;x++)
	{
		switch(bmp->depth)
		{
			case 32: s = data[lineoffset*y+(x<<2)+1] != 0; break;
			case 24: s = data[lineoffset*y+x*3] != 0; break;
			case 16: s = data[lineoffset*y+(x<<1)+1]&32 != 0; break;
			case 1:  s = ~data[lineoffset*y+(x>>3)]&(128>>(x&7)); break;
		}

		if(s) address[((18-y) << 2) + (x >> 3)] |= (128 >> (x & 7));
		else address[((18-y) << 2) + (x >> 3)] &= ~(128 >> (x & 7));

//		char c = s ? '#' : ' ';
//		fputc(c,stdout), fputc(c,stdout);
	}

	*address = 0;
	return;
}
