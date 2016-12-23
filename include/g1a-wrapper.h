#ifndef _G1A_WRAPPER_H
	#define _G1A_WRAPPER_H 1

/*
	Standard header inclusions.
*/

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



/*
	Composed types definitions.
*/

// Options management structure.
struct Options
{
	// Is the actio to dump a file ?
	int dump;
	// Input and output file names.
	char *input;
	char *output;
	// Is the output file name dynamcally allocated ?
	int output_dynamic;
	// Program name, version, internal name, build date.
	char name[9];
	char version[11];
	char internal[9];
	char date[15];
	// Raw monochrome icon data.
	uint8_t icon[76];
};

/* File header structure.
struct G1A_Header
{ 
	uint8_t magic[8];
	uint8_t addin_id;
	uint8_t unknown1[5];
	uint8_t control1;
	uint8_t unknown2;
	uint8_t filesize_uint[4];
	uint8_t control2;
	uint8_t custom_seq[11];
	uint8_t internal[8];
	uint8_t gap1[3];
	uint8_t estrips;
	uint8_t gap2[4];
	uint8_t version[10];
	uint8_t gap3[2];
	uint8_t date[14];
	uint8_t gap4[2];
	uint8_t bitmap[68];
	uint8_t estrip1[80];
	uint8_t estrip2[80];
	uint8_t estrip3[80];
	uint8_t estrip4[80];
	uint8_t gap5[4];
	uint8_t name[8];
	uint8_t gap6[20];
	uint8_t filesize_ulong[4];
	uint8_t gap7[12];
}; */



/*
	Function prototypes.
*/

// Main function.
int main(int argc, char **argv);
// Generating options structure from command-line arguments.
void args(int argc, char **argv, struct Options *options);
// Generating header data from options.
void generate(struct Options options, unsigned char *data);
// Writing header data and binary content to file.
void write(const char * inputfile, const char *outputfile,
	unsigned char *data);

// Testing if a string matches a simple format.
int string_format(const char *str, const char *format);

// Dumping a g1a file's header content.
void dump(const char *filename);
// Displaying program help.
void help(void);
// Displaying header information.
void info(void);

#endif // _G1A_WRAPPER_H
