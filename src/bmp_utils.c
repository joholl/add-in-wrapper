/**************************************************************************
    This file is part of c_g1awrapper.

    c_g1awrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    c_g1awrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with c_g1awrapper.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include "bmp_utils.h"
#include <stdio.h>
#include <stdlib.h>

int readBMP(struct BMP_File *bmp, FILE *file)
{
	unsigned char buffer[128];
	unsigned int offset;
	unsigned int tmp, rawsize;
	int w, h;
	int bpp;
	
	if(fread(buffer, sizeof(unsigned char), 2, file) != 2) return -1;

	if((buffer[0]!='B' || buffer[1]!='M') &&
		 (buffer[0]!='B' || buffer[1]!='A') &&
		 (buffer[0]!='C' || buffer[1]!='I') &&
		 (buffer[0]!='C' || buffer[1]!='P') &&
		 (buffer[0]!='I' || buffer[1]!='C') &&
			 (buffer[0]!='P' || buffer[1]!='T')) return -1;

	fseek(file, 8, SEEK_CUR);
	if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
	offset = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
	printf("offset : %d\n", offset);

	if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
	tmp = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);

	if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
	w = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
	if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
	h = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
	printf("image dimensions : (%d,%d)\n", w, h);
		
	fseek(file, 2, SEEK_CUR);
	if(fread(buffer, sizeof(unsigned char), 2, file) != 2) return -1;
	bpp = (buffer[1]<<8) + (buffer[0]);

	printf("bit per pixel : %d\n", bpp);
	fseek(file, 4, SEEK_CUR);
	if(fread(buffer, sizeof(unsigned char), 4, file) != 4) return -1;
	rawsize = (buffer[3]<<24) + (buffer[2]<<16) + (buffer[1]<<8) + (buffer[0]);
	tmp = ((bpp * w - 1)/32 + 1)*4 * h;
	printf("raw size : %d (computed : %d)\n", rawsize, tmp);
	if(rawsize != tmp) return -2;

	fseek(file, offset, SEEK_SET);
	if(!(bmp->bitmap = calloc(rawsize,sizeof(unsigned char)))) return -1;
	if(fread(bmp->bitmap, sizeof(unsigned char), rawsize, file) != rawsize) {
		free(bmp->bitmap);
		return -1;
		printf("Error when reading the raw bitmap.\n"); }

	bmp->width = w;
	bmp->height = h;
	bmp->bitPerPixel = bpp;
	return 0;
}

void getMonoBitmap(const struct BMP_File *bmp, unsigned char *bitmap)
{
	int i, j;
	int lineSize	= (((bmp->bitPerPixel*bmp->width-1)/32)+1)*4;
	int bLineSize = ((bmp->width-1)/8)+1;
	int rowOffset;
		int pixel; // 0 is black

	for(i=0; i < bLineSize * bmp->height; i++) bitmap[i] = 0x00;
	
	for(i=0; i < bmp->height; i++) {
		rowOffset = lineSize*(bmp->height - i - 1);
		for(j=0; j < bmp->width; j++) {
			pixel = 1;
			switch(bmp->bitPerPixel) {
				case 1: pixel = !(bmp->bitmap[rowOffset + j/8] & (0x01 << (7-(j%8)))); break;
				case 2: pixel = !(bmp->bitmap[rowOffset + j/4] & (0x03 << (2*(3-(j%4))))); break;
				case 4: pixel = !(bmp->bitmap[rowOffset + j/2] & (0x07 << (4*(1-(j%2))))); break;
				case 8: pixel = !(bmp->bitmap[rowOffset + j]); break;
				case 24: pixel = ((bmp->bitmap[rowOffset + 3*j] & bmp->bitmap[rowOffset + 3*j+1] & bmp->bitmap[rowOffset + 3*j+2]) == 0xFF); break;
				case 32: pixel = ((bmp->bitmap[rowOffset + 4*j] | bmp->bitmap[rowOffset + 4*j+1] | bmp->bitmap[rowOffset + 4*j+2]) == 0xFF); break; }
			if(pixel == 0) bitmap[bLineSize*i + (j>>3)] |= 0x1 << (7-(j&7));
		}
	}
}

void printBitmap(FILE *stream, unsigned char *bitmap, int w, int h)
{
	int bLineSize = (w-1)/8+1;
	int i, j;

	for(i=0; i<h; i++, fputc('\n',stream)) for(j=0; j<w; j++)
	{
		if(bitmap[bLineSize*i + j/8] & (0x1 << (7-(j%8)))) {
			fputc('#', stream);
			fputc('#', stream); }
		else {
			fputc(' ', stream);
			fputc(' ', stream); }
	}

}
