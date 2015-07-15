/*********************************************
**
**  Main source file of g1a-wrapper tool.
**
*********************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "g1a-wrapper.h"
#include "error.h"
#include "bmp_utils.h"

extern const char *_help_string;

int main(int argc, char **argv)
{
	// Header structure, options structure, error indicator.
	struct G1A_Header header;
	struct Options options;
	int failure=0;

	//Initializing error module.
	error_init("g1a-wrapper",1,&failure);
	error_add(FATAL,"no-input","no input file");
	error_add(FATAL,"input","cannot open input file '%s' for reading");
	error_add(FATAL,"output","cannot open output file '%s' for writing");
	error_add(ERROR,"~option","unrecognized option '%s'");
	error_add(ERROR,"~illegal","unexpected token '%s'");
	error_add(ERROR,"bmp-no-open","cannot open input bitmap file '%s' for reading");
	error_add(ERROR,"bmp-alloc","cannot allocate memory to read bitmap file '%s'");
	error_add(ERROR,"bmp-valid","file '%s' is not a valid bmp file");
	error_add(ERROR,"bmp-depth","bitmap image '%s' has unsupported depth %d");
	error_add(WARNING,"~format","%s '%s' does not have expected format '%s'");
	error_add(WARNING,"~length","%s '%s' is more than %d characters long");
	error_add(WARNING,"~bmp-width","bitmap image '%s' has width %d, expected %d");
	error_add(WARNING,"~bmp-height","bitmap image '%s' has height %d, expected %d");

	// Parsing command-line arguments.
	args(argc,argv,&options);

	// If an error occurred, quits the program.
	if(failure) return 1;

	// Generating the header according to the command-line parameters.
	generate(options,(char *)&header);

	// Writing the header and the binary content.
	write(options.input,options.output,(char *)&header);

	return 0;
}

void generate(struct Options options, char *data)
{
	unsigned char unknown1[5] = { 0x00,0x10,0x00,0x10,0x00 };
	int i;

	strncpy(data,"USBPower",8);
	data[0x008] = 0xF3;
	for(i=0;i<5;i++) data[0x009+i] = unknown1[i];
	data[0x00F] = 0x01;
	for(i=0;i<11;i++) data[0x015+i] = 0x00;
	strncpy(data+0x20,options.internal,8);
	for(i=0x028;i<0x02B;i++) data[i] = 0;
	data[0x02B] = 0;
	for(i=0x02C;i<0x030;i++) data[i] = 0;
	strncpy(data+0x030,options.version,10);
	for(i=0x03A;i<0x03C;i++) data[i] = 0;
	strncpy(data+0x03C,options.date,14);
	for(i=0x04A;i<0x04C;i++) data[i] = 0;
	memcpy(data+0x04C,options.icon+4,68);
	for(i=0x090;i<0x1D0;i++) data[i] = 0;
	for(i=0x1D0;i<0x1D4;i++) data[i] = 0;
	for(i=0x1DC;i<0x1F0;i++) data[i] = 0;
	strncpy(data+0x1D4,options.name,8);
	for(i=0x1F4;i<0x200;i++) data[i] = 0;
}

void write(const char *inputfile, const char *outputfile, char *data)
{
	FILE *input = fopen(inputfile,"rb");
	FILE *output = fopen(outputfile,"wb");
	unsigned char byte;
	unsigned int size;
	int i;

	if(!input) error_emit(FATAL,"input",inputfile);
	if(!output) error_emit(FATAL,"output",outputfile);

	fseek(input,0,SEEK_END);
	size = ftell(input)+0x200;
	fseek(input,0,SEEK_SET);

	data[0x00E] = size + 0x41;
	data[0x014] = size + 0xB8;
	for(i=0;i<4;i++) *(data+0x010+i) = *(((char *)&size)+3-i);
	for(i=0;i<4;i++) *(data+0x1F0+i) = *(((char *)&size)+3-i);

	for(i=0;i<0x015;i++) data[i] = ~data[i];
	fwrite(data,1,sizeof(struct G1A_Header),output);

	while(fread(&byte,1,1,input)) fwrite(&byte,1,1,output);

	fclose(input);
	fclose(output);
}

void args(int argc, char **argv, struct Options *options)
{
	// Declaring variables.
	int i;

	// Initializing option values.
	options->input = NULL;
	options->output = NULL;
	*options->name = 0;
	strcpy(options->version,"00.00.0000");
	strcpy(options->internal,"@ADDIN");
	*options->date = 0;

	// Parsing the loop to detect the error parameters.
	for(i=1;i<argc;i++) error_argument(argv[i]);

	// Parsing the different given parameters.
	for(i=1;i<argc;i++)
	{
		// Help command.
		if(!strcmp(argv[i], "-h") || !strcmp(argv[i],"--help")) help();
		// Format command.
		if(!strcmp(argv[i], "-f") || !strcmp(argv[i],"--format"))
			format();

		// Output filename.
		if(!strcmp(argv[i],"-o")) options->output = argv[++i];
		else if(!strncmp(argv[i],"--output=",9)) options->output = argv[i]+9;

		// Application name.
		else if(!strcmp(argv[i],"-n") || !strncmp(argv[i],"--name=",7))
		{
			char *name = argv[i][1]=='-' ? argv[i]+7 : argv[++i];
			if(string_cal(options->name,name,8))
				error_emit(WARNING,"length","application name",name,8);
		}

		// Program icon.
		else if(!strcmp(argv[i],"-i") || !strncmp(argv[i],"--icon=",7))
		{
			char *iconfile = argv[i][1]=='-' ? argv[i]+7 : argv[++i];
			bitmap_read(iconfile,30,19,options->icon);
		}

		// Program version.
		else if(!strcmp(argv[i],"-v") || !strncmp(argv[i],"--version=",10))
		{
			char *version = argv[i][1]=='-' ? argv[i]+10 : argv[++i];
			if(string_cal(options->version,version,10))
				error_emit(WARNING,"length","version string",version,10);
			else if(string_format(options->version,"00.00.0000"))
				error_emit(WARNING,"format","version string",options->version,"MM.mm.pppp");
		}

		// Build date.
		else if(!strcmp(argv[i],"-d") || !strncmp(argv[i],"--date=",7))
		{
			char *date = argv[i][1]=='-' ? argv[i]+7 : argv[++i];
			if(string_cal(options->date,date,14))
				error_emit(WARNING,"length","date string",date,14);
			else if(string_format(options->date,"0000.0000.0000"))
				error_emit(WARNING,"format","date string",options->date,"yyyy.MMdd.hhmm");
		}

		// Internal application name.
		else if(!strcmp(argv[i],"-N") || !strncmp(argv[i],"--internal=",11))
		{
			char *internal = argv[i][1]=='-' ? argv[i]+11 : argv[++i];
			if(string_cal(options->internal,internal,8))
				error_emit(WARNING,"length","internal name",internal,8);
			else if(string_format(options->internal,"@AAAAAAA"))
				error_emit(WARNING,"format","internal name",options->internal,"@[A-Z]{0,7}");
		}

		// Testing if the argument is enabling or disabling some errors.
		else if(!error_argument(argv[i]));

		// Looking for an unrecognized option.
		else if(*(argv[i])=='-') error_emit(ERROR,"option",argv[i]);

		// Everything else is considered as the binary file name.
		else
		{
			if(options->input) error_emit(ERROR,"illegal",argv[i]);
			options->input = argv[i];
		}
	}

	// Testing if a input binary file was given.
	if(!options->input) error_emit(FATAL,"no-input");

	// Setting the default output filename if no one was given.
	if(!options->output)
	{
		char *tmp = strrchr(options->input,'.');
		int length;
		if(!tmp) tmp = options->input + strlen(options->input);
		length = tmp - options->input;
		options->output = malloc(length+5);
		strncpy(options->output,options->input,length);
		strcpy(options->output+length,".g1a");
	}

	// Setting the default filename if no one was given.
	if(!*options->name)
	{
		char *tmp = strrchr(options->output,'.');
		int length;
		if(!tmp) tmp = options->output + strlen(options->output);
		length = tmp - options->output;
		if(length>8) length=8;
		strncpy(options->name,options->output,length);
		options->name[length] = 0;
	}

	// Setting the default build date if no one was given.
	if(!*options->date)
	{
		time_t rawtime;
		struct tm *timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		sprintf(options->date,"%04d.%02d%02d.%02d%02d",
			timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min);
	}
}

int string_cal(char *dest, const char *src, size_t maxlength)
{
	/*
	**  String manipulation additional function : copy and limit.
	**  Copies the string and limits the size to the given limit.
	**  Returns 1 if the string was shortened, or 0 in any other case.
	*/

	// These are return value, incremental counter, effective size to copy.
	int x=0, i=0;
	int length=0;

	// Gets source string length.
	while(src[length]) length++;

	// Adjusts the size to copy.
	if((size_t)length > maxlength) length=maxlength, x=1;

	// Copies the string and adds a null-terminating character.
	while(i<length) dest[i] = src[i], i++;
	dest[length] = 0;

	// Returns from subroutine.
	return x;
}

int string_format(const char *str, const char *format)
{
	/*
	**  String manipulation additional function : format.
	**  Checks if a given string matches a template. It is not as complex
	**  as regexs, it just checks characters one by one. If format is shorter
	**  than str, the function returns 1. If str is shorter, the function
	**  returns the result of the comparison.
	**  Returns 0 if str matches the given format.
	**  'a' = [a-z], 'A' = [A-Z], '0' = [0-9], '*' = everything printable.
	**  Every other character should be exactly the same.
	*/

	while(*str && *format)
	{
		switch(*format)
		{
			case 'a': if(!islower(*str)) return 1; break;
			case 'A': if(!isupper(*str)) return 1; break;
			case '0': if(!isdigit(*str)) return 1; break;
			case '*': if(!isprint(*str)) return 1; break;
			default : if(*str!=*format)  return 1; break;
		}

		str++;
		format++;
	}

	return (*format==0 && *str);
}

void help(void)
{
	puts(_help_string);

	exit(0);
}

void format(void)
{
	printf("g1a header data format :\n\n");
	printf("0x%03x magic\n",
		(unsigned int)offsetof(struct G1A_Header, magic));
	printf("0x%03x addin_id\n",
		(unsigned int)offsetof(struct G1A_Header, addin_id));
	printf("0x%03x control1\n",
		(unsigned int)offsetof(struct G1A_Header, control1));
	printf("0x%03x filesize_uint\n",
		(unsigned int)offsetof(struct G1A_Header, filesize_uint));
	printf("0x%03x control2\n",
		(unsigned int)offsetof(struct G1A_Header, control2));
	printf("0x%03x custom_seq\n",
		(unsigned int)offsetof(struct G1A_Header, custom_seq));
	printf("0x%03x internal\n",
		(unsigned int)offsetof(struct G1A_Header, internal));
	printf("0x%03x estrips\n",
		(unsigned int)offsetof(struct G1A_Header, estrips));
	printf("0x%03x version\n",
		(unsigned int)offsetof(struct G1A_Header, version));
	printf("0x%03x date\n",
		(unsigned int)offsetof(struct G1A_Header, date));
	printf("0x%03x bitmap\n",
		(unsigned int)offsetof(struct G1A_Header, bitmap));
	printf("0x%03x estrip1\n",
		(unsigned int)offsetof(struct G1A_Header, estrip1));
	printf("0x%03x estrip2\n",
		(unsigned int)offsetof(struct G1A_Header, estrip2));
	printf("0x%03x estrip3\n",
		(unsigned int)offsetof(struct G1A_Header, estrip3));
	printf("0x%03x estrip4\n",
		(unsigned int)offsetof(struct G1A_Header, estrip4));
	printf("0x%03x name\n",
		(unsigned int)offsetof(struct G1A_Header, name));
	printf("0x%03x filesize_ulong\n",
		(unsigned int)offsetof(struct G1A_Header, filesize_ulong));

	exit(0);
}
