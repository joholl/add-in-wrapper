/*********************************************
**
**  Main header file of g1a-wrapper tool.
**
*********************************************/

#ifndef _G1A_WRAPPER_H
	#define _G1A_WRAPPER_H 1

// Type definition.
	#ifndef _UINT8_T
		typedef unsigned char uint8_t;
	#endif // _UINT8_T

// Options management structure definition.
	struct Options
	{
		char *input;
		char *output;
		char name[9];
		char version[11];
		char internal[9];
		char date[15];
		uint8_t icon[76];
	};

/*
Offset	Size	Description
0x000	8		"USBPower"
0x008	1		0xF3 (AddIn)
0x009	5		{ 0x00,0x10,0x00,0x10,0x00 }
0x00E	1		@0x13 + 0x41
0x00F	1		0x01
0x010	4		filesize: uint, big endian
0x014	1		@0x13 + 0xB8
0x015	9	1	[Unsignificant]
0x01E	2	1	Number of objects (if MCS)
0x020	8	+	Internal name `@APPNAME`
0x028	3		-
0x02B	1	1	Number of estrips
0x02C	4		-
0x030	10	+	Version `01.23.4567`
0x03A	2		-
0x03C	14	+	Date `yyyy.MMdd.hhmm`
0x04A	2		-
0x04C	68	1	30×17 icon.
0x090	80	1	eStrip 1
0x0E0	80	1	eStrip 2
0x130	80	1	eStrip 3
0x180	80	1	eStrip 4
0x1D0	4		-
0x1D4	8	+	Program name
0x1DC	20		-
0x1F0	4		filesize: ulong, big endian
0x1F4	12		-
0x200	...		Addin code
*/

// File header structure definition.
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
	};

// Function prototypes.
	int main(int argc, char ** argv);
	void generate(struct Options options, char *data);
	void write(const char * inputfile, const char *outputfile, char *data);
	void args(int argc, char **argv, struct Options *options);
	int string_cal(char *dest, const char *src, size_t maxlength);
	int string_format(const char *str, const char *format);
	void help(void);
	void format(void);

#endif // _G1A_WRAPPER_H
