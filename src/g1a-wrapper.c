/*
	Header inclusions.
*/

#include "g1a-wrapper.h"
#include "error.h"
#include "bmp_utils.h"

/*
	main()

	Program main function. Initializes the error module and calls various
	subroutines to read the command-line options, generate the header and
	write it.

	@arg	argc	Command-line argument count.
	@arg	argv	NULL-terminated command-line arguments array.

	@return		Error code.
*/

int main(int argc, char **argv)
{
	/*
		Error definitions.
	*/

	// Fatal errors.
	const char *fatals[] = {
		// No binary input file provided.
		"no-input", "no input file",
		// Input file cannot be read.
		"input", "cannot open input file '%s' for reading",
		// Output file cannot be written.
		"output", "cannot open output file '%s' for writing",
		// NULL terminator.
		NULL
	};

	// Standard errors (those who begin with '~' can be masked).
	const char *errors[] = {
		// Unrecognized command-line option found.
		"~option", "unrecognized option '%s'",
		// Illegal invocation syntax (unexpected option).
		"~illegal", "unexpected token '%s'",
		// Alloc failure.
		"alloc", "alloc failure (not enough resources)",
		// Bitmap file cannot be open.
		"bmp-no-open", "cannot open bitmap file '%s' for reading",
		// Provided file is not a valid bitmap.
		"bmp-valid", "file '%s' is not a valid bmp file",
		// Bitmap format is not supported.
		"bmp-depth", "bitmap image '%s' has unsupported depth %d",
		// The given file to dump is not a valid g1a file.
		"g1a-valid", "file '%s' is not a valid g1a file (%s)",
		// NULL terminator.
		NULL
	};

	// Warnings (those who begin with '~' can be masked).
	const char *warnings[] = {
		// A field parameter is too long.
		"~length", "%s '%s' is too long (maximum is %d characters)",
		// A field hasn't the advised format.
		"~format", "%s '%s' does not have expected format '%s'",
		// The given bitmap image hasn't the right width.
		"~bmp-width", "bitmap image '%s' has width %d, expected %d",
		// The given bitmap image hasn't the right height.
		"~bmp-height", "bitmap image '%s' has height %d, expected %d",
		// The given bitmap is not made only of black and white pixels.
		"~bmp-color", "bitmap image '%s' is not black and white",
		// 16-bit bitmaps are not fully supported.
		"bmp-16-bit", "16-bit bitmap '%s' is not fully supported",
		// NULL terminator.
		NULL
	};

	// Using a header structure, used by pointer.
	struct G1A_Header header;
	// Using an options structure.
	struct Options options;
	// Using a failure indicator.
	int failure = 0;
	// Using an iterator.
	int i;

	//Initializing error module.
	error_init("g1a-wrapper", 1, &failure);

	// Initializing various fatal errors, errors, and warnings.
	for(i = 0; fatals[i]; i+=2) error_add(FATAL, fatals[i], fatals[i+1]);
	for(i = 0; errors[i]; i+=2) error_add(ERROR, errors[i], errors[i+1]);
	for(i = 0; warnings[i]; i+=2)
		error_add(WARNING, warnings[i], warnings[i+1]);

	// Parsing command-line arguments.
	args(argc, argv, &options);

	// If an error occurred, returning from the program.
	if(failure) return 1;

	// Dumping input file if the dump option has been activated. Then,
	// returning the program.
	if(options.dump)
	{
		// Dumping the file.
		dump(options.input);
		// Returning the program.
		return 0;
	}

	// Generating the header according to the command-line parameters.
	generate(options, (char *)&header);

	// Writing the header and the binary content.
	write(options.input, options.output, (char *)&header);

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
	strncpy(data+0x1D4,options.name,8);
	for(i=0x1DC;i<0x200;i++) data[i] = 0;
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
	for(i=0; i<4; i++) data[0x010+i] = data[0x1f0+i] = *(((char *)&size)+3-i);

	for(i=0;i<0x020;i++) data[i] = ~data[i];
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
	options->dump = 0;
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
		// Info command.
		if(!strcmp(argv[i],"--info")) info();

		if(!strcmp(argv[i],"-d"))
		{
			options->input = argv[++i];
			options->dump = 1;
			continue;
		}

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

		// Testing if the argument is disabling some errors.
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

/*
	sring_format()

	This simple function checks if a string matches the given fixed-length
	format.
	
*/

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
			default: if(*str != *format) return 1; break;
		}

		str++;
		format++;
	}

	return (*format==0 && *str);
}

/*
	dump()

	Dumps the file header contents, assuming the file is a g1a file.

	@arg	filename	File to dump header.
*/

void dump(const char *filename)
{
	// Using an array to store header data.
	char data[0x200];
	// Using a file pointer to read file contents.
	FILE *fp;
	// Using an integer to store the total file size and a temporary
	// integers.
	int filesize, t1, t2;
	// Using an iterator.
	int i;
	// Using a parsing pointer.
	char *ptr;

	// Opening file.
	fp = fopen(filename, "r");
	// Handling failure by emitting a fatal error.
	if(!fp) error_emit(FATAL, "input", filename);
	// Reading file header contents.
	fread(data, 0x200, 1, fp);
	// Retrieving the file size.
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	// Closing the file.
	fclose(fp);

	// Inverting the general header !
	for(i=0; i < 0x020; i++) data[i] = ~data[i];



	/*
		Checking file validity.
	*/

	// A g1a must have binary code with its header !
	if(filesize <= 0x200)
	{
		// Emitting an error if there's not binary code.
		error_emit(ERROR, "g1a-valid", filename, "too short");
		// Then returning : why would we analyze an non-g1a file ?
		return;
	}

	// Looking for initial string "USBPower".
	if(strncmp(data, "USBPower", 8))
	{
		// Emitting an error if not found.
		error_emit(ERROR, "g1a-valid", filename, "\"USBPower\"");
		return;
	}

	// Looking for MCS add-in indicator.
	if((unsigned char )data[8] != 0xf3)
	{
		// Emitting an error.
		error_emit(ERROR, "g1a-valid", filename, "not an add-in");
		return;
	}

	// Looking for the file size.
	t1 = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
	t2 = (data[496] << 24) | (data[497] << 16) | (data[498] << 8)
		| data[499];
	// Checking validity.
	if(t1 != filesize || t2 != filesize)
	{
		// Emitting an error.
		error_emit(ERROR, "g1a-valid", filename, "wrong file size");
		return;
	}

	// Getting the checksums.
	t1 = (data[19] + 0x41) & 0xff;
	t2 = (data[19] + 0xb8) & 0xff;
	// Checking checksums.
	if((uint8_t)data[14] != t1 || (uint8_t)data[20] != t2)
	{
		// Emitting an error message.
		error_emit(ERROR, "g1a-valid", filename, "wrong checksums");
		return;
	}

	// Printing the input file name.
	printf("Input file     '%s'\n", filename);
	// Printing the input file size.
	printf("File size       %d bytes\n\n", filesize);

	// Printing the program name.
	printf("Program name   '");
	ptr = data + 0x1d4;
	// Printing the header characters and a line break.
	while(ptr < data + 0x1dc && *ptr) putchar(*ptr++);
	puts("'");

	// Printing the program internal name.
	printf("Internal name  '");
	ptr = data + 0x020;
	// Printing the header characters and a line break.
	while(ptr < data + 0x028 && *ptr) putchar(*ptr++);
	puts("'");

	// Printing the program version.
	printf("Version        '");
	ptr = data + 0x030;
	// Printing the header characters and a line break.
	while(ptr < data + 0x03a && *ptr) putchar(*ptr++);
	puts("'");

	// Printing the program build date.
	printf("Build data     '");
	ptr = data + 0x03c;
	// Printing the header characters and a line break.
	while(ptr < data + 0x04a && *ptr) putchar(*ptr++);
	puts("'");
}

/*
	help()

	Prints a help message and exits.
*/

void help(void)
{
	// Printing a help message.
	puts(
"Usage: g1a-wrapper <bin_file> [options]\n"
"\n"
"g1a-wrapper creates a g1a file (add-in application for CASIO fx-9860G\n"
"calculator series) from the given binary file and options.\n"
"\n"
"Available options :\n"
"  -o, --output=<file>  Output file name. Default is 'addin.g1a'.\n"
"  -i, --icon=<bitmap>  Program icon, must be a valid non-indexed bmp file.\n"
"                       Default is a blank icon.\n"
"  -n, --name=<name>    Name of the add-in application. At most 8 characters.\n"
"                       Default is the truncated output filename.\n"
"  -v  --version=<text> Program version. Format 'MM.mm.pppp' advised. Default\n"
"                       is '00.00.0000'.\n"
"  -N, --internal=<name>Internal name of the program. Uppercase and '@' at\n"
"                       beginning advised. Default is '@ADDIN'.\n"
"  -d, --date=<date>    Date of the build, using format 'yyyy.MMdd.hhmm'.\n"
"                       Default is the current time.\n"
"\n"
"Other options :\n"
"  -h, --help           Displays this help.\n"
"  --info               Displays header format information.\n"
"\n"
"You can disable warnings during program execution.\n"
"\n"
"Warning options :\n"
"  -Wlength     One of the parameters is too long and will be truncated.\n"
"  -Wformat     The parameter doesn't fit the default advised format.\n"
"\n"
"Some errors can also be masked. However, forcing execution of the wrapper can\n"
"lead to the generation of an incorrect g1a file.\n"
"\n"
"Error options :\n"
"  -Eoption     Unrecognized option found.\n"
"  -Eillegal    Illegal invocation syntax (unexpected option found).\n"
	);

	// Exiting the program.
	exit(0);
}

/*
	info()

	Outputs informations about the header file format and exits.
*/

void info(void)
{
	// Printing header file format informations.
	puts(
		"Add-in header format :\n"
		"\n"
		"Offset	Size	Description\n"
		"0x000	8	\"USBPower\"\n"
		"0x008	1	0xF3 (AddIn)\n"
		"0x009	5	{ 0x00, 0x10, 0x00, 0x10, 0x00 }\n"
		"0x00E	1	@0x13 + 0x41\n"
		"0x00F	1	0x01\n"
		"0x010	4	File size: unsigned int, big endian\n"
		"0x014	1	@0x13 + 0xB8\n"
		"0x015	9	[Unsignificant]\n"
		"0x01E	2	Number of objects (if MCS)\n"
		"0x020	8	Internal name '@APPNAME'\n"
		"0x028	3	-\n"
		"0x02B	1	Number of estrips\n"
		"0x02C	4	-\n"
		"0x030	10	Version 'MM.mm.pppp'\n"
		"0x03A	2	-\n"
		"0x03C	14	Date 'yyyy.MMdd.hhmm'\n"
		"0x04A	2	-\n"
		"0x04C	68	30*17 icon.\n"
		"0x090	80	eStrip 1\n"
		"0x0E0	80	eStrip 2\n"
		"0x130	80	eStrip 3\n"
		"0x180	80	eStrip 4\n"
		"0x1D0	4	-\n"
		"0x1D4	8	Program name\n"
		"0x1DC	20	-\n"
		"0x1F0	4	File size: unsigned long, big endian\n"
		"0x1F4	12	-\n"
		"0x200	...	Binary content\n"
	);

	// Exiting the program.
	exit(0);
}
