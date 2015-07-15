/*
	g1a-wrapper

	Little command-line program that generates g1a file headers and appends
	binary file content to output a full g1a file.
*/



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

	const char *notes[] = {
		// Default value used.
		"~default", "No %s provided, falling back to '%s'",
		// NULL terminator.
		NULL
	};

	// Using memory for a header.
	char header[0x200];
	// Using an options structure.
	struct Options options;
	// Using a failure indicator.
	int failure = 0;
	// Using an iterator.
	int i;

	//Initializing error module.
	error_init("g1a-wrapper", 1, &failure);

	// Initializing various fatal errors, errors, warnings and notes.
	for(i = 0; fatals[i]; i+=2) error_add(FATAL, fatals[i], fatals[i+1]);
	for(i = 0; errors[i]; i+=2) error_add(ERROR, errors[i], errors[i+1]);
	for(i = 0; warnings[i]; i+=2)
		error_add(WARNING, warnings[i], warnings[i+1]);
	for(i = 0; notes[i]; i+=2) error_add(NOTE, notes[i], notes[i+1]);

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
	generate(options, header);

	// Writing the header and the binary content.
	write(options.input, options.output, header);

	// Freeing the output file name field if it was dynamically allocated.
	if(options.output_dynamic) free(options.output);

	// Successfully returning from the program.
	return 0;
}

/*
	args()

	Parses the command-line arguments and fills the options structure
	according to their meanings.

	@arg	argc	Number of command-line arguments.
	@arg	argv	NULL-terminated array of command-line arguments.
	@arg	options	Options structure pointer to fill.
*/

void args(int argc, char **argv, struct Options *options)
{
	// Using default icon data.
	uint8_t default_icon_1[] = { 0x00, 0x00, 0x00, 0x04 };
	uint8_t default_icon_2[] = { 0x00, 0x00, 0x01, 0xfc };
	// Using an iterator to parse the various arguments.
	int i;

	/*
		Initializing values.
	*/

	// By default, action is to wrap, not to dump.
	options->dump = 0;
	// No default file specified.
	options->input = NULL;
	options->output = NULL;
	// The output file name wasn't dynamically allocated, for now.
	options->output_dynamic = 0;
	// Empty program name and build date.
	*options->name = 0;
	*options->date = 0;
	// Default version and internal name, as said in the help page.
	strcpy(options->version, "00.00.0000");
	strcpy(options->internal, "@ADDIN");
	// Initializing the icon. Copying 12 lines of pattern 1 (the first will
	// be omitted when assembling the g1a file).
	for(i = 0; i < 12; i++)
		memcpy(options->icon + (i << 2), default_icon_1, 4);
	// Then copying 6 lines of pattern 2 (no need to append a last line).
	for(i = 12; i < 19; i++)
		memcpy(options->icon + (i << 2), default_icon_2, 4);

	// Parsing the loop to detect the error parameters.
	for(i = 1; i < argc; i++)
	{
		// If the argument show an error, mask it.
		if(!error_argument(argv[i])) argv[i] = NULL;
	}

	// Parsing the different given parameters.
	for(i = 1; i < argc; i++)
	{
		// Skipping NULL arguments.
		if(!argv[i]) continue;



		/*
			Handling commands.
		*/

		// Handling command -h, --help : help page.
		if(!strcmp(argv[i], "-h") || !strcmp(argv[i],"--help")) help();
		// Handling command --info : data header information.
		if(!strcmp(argv[i],"--info")) info();
		// Handling command -d : g1a file dump.
		if(!strcmp(argv[i],"-d"))
		{
			// Setting the dump option.
			options->dump = 1;
			// Incrementing parsing index. The input filename will
			// be automatically set.
			i++;
		}



		/*
			Handling general options.
		*/

		// Handling option -o : output file name.
		if(!strcmp(argv[i],"-o")) options->output = argv[++i];

		// Handling option -n : application name.
		else if(!strcmp(argv[i],"-n"))
		{
			// Getting and copying the program name.
			char *name = argv[++i];
			strncpy(options->name, name, 8);
			// Emitting a length warning if it exceeds 8 bytes.
			if(strlen(name) > 8) error_emit(WARNING, "length",
				"application name", name, 8);
		}

		// Handling option -i : program icon.
		else if(!strcmp(argv[i],"-i"))
		{
			// Reading the bitmap data (this is a heavy procedure).
			bitmap_read(argv[++i], 30, 19, options->icon);
		}



		/*
			Handling advanced options.
		*/

		// Handling option --version : program version.
		else if(!strncmp(argv[i],"--version=",10))
		{
			// Getting the version string.
			char *version = argv[i] + 10;
			// Copying it to the corresponding field.
			strncpy(options->version, version, 10);

			// Emitting a warning if it's too long.
			if(strlen(version) > 10) error_emit(WARNING, "length",
				"version string", version,10);
			// Or if it doesn't matches the default format.
			else if(string_format(version,"00.00.0000"))
				error_emit(WARNING, "format", "version string",
				options->version,"MM.mm.pppp");
		}

		// Handling option --date : build date.
		else if(!strncmp(argv[i], "--date=", 7))
		{
			// Getting the date string.
			char *date = argv[i] + 7;
			// Copying it to the right field.
			strncpy(options->date, date, 14);

			if(strlen(date) > 14) error_emit(WARNING, "length",
				"date string", date, 14);
			else if(string_format(date,"0000.0000.0000"))
				error_emit(WARNING, "format", "date string",
				options->date, "yyyy.MMdd.hhmm");
		}

		// Handling option --internal : internal program name.
		else if(!strncmp(argv[i], "--internal=", 11))
		{
			// Getting the internal name.
			char *internal = argv[i] + 11;
			// Copying it to its field.
			strncpy(options->internal, internal, 8);

			if(strlen(internal) > 8) error_emit(WARNING, "length",
				"internal name", internal, 8);
			else if(string_format(options->internal,"@AAAAAAA"))
				error_emit(WARNING, "format", "internal name",
				options->internal, "@[A-Z]{0,7}");
		}

		// Looking for an unrecognized option.
		else if(*(argv[i]) == '-')
		{
			// Emitting an error containing the argument.
			error_emit(ERROR, "option", argv[i]);
		}

		// Everything else is considered as the binary file name.
		else
		{
			// If there is already an input file.
			if(options->input)
			{
				// Emitting an error.
				error_emit(ERROR, "illegal", argv[i]);
				// Continuing to prevent re-assignment.
				continue;
			}

			// Setting the input file name.
			options->input = argv[i];
		}
	}

	// Testing if a input binary file was given.
	if(!options->input) error_emit(FATAL, "no-input");

	// Skipping all those default values if the wanted action is to dump
	//a g1a file.
	if(options->dump) return;

	// Setting the default output filename if no one was given.
	if(!options->output)
	{
		// Looking for the dot.
		char *tmp = strrchr(options->input,'.');
		// Using an integer to store the filename base length.
		int length;

		// If no dot is found, going to the string end.
		if(!tmp) tmp = options->input + strlen(options->input);
		// Computing the base name length.
		length = tmp - options->input;

		// Allocating data to output.
		options->output = malloc(length + 5);
		// As a consequence, setting the flag not to forget freeing
		// this field.
		options->output_dynamic = 1;
		// Copying the base name.
		strncpy(options->output, options->input, length);
		// Appending extension '.g1a'.
		strcpy(options->output + length, ".g1a");

		// Emitting a note.
		error_emit(NOTE, "default", "output filename",
			options->output);
	}

	// Setting the default filename if no one was given.
	if(!*options->name)
	{
		// Looking for the dot in the binary file name.
		char *dot = strrchr(options->output, '.');
		// Looking for the slash.
		char *sla = strrchr(options->output, '/');
		// Using an integer to store the length of the base name.
		int length;

		// If no slash nor dot is found, include the whole name.
		if(!dot) dot = options->output + strlen(options->output);
		if(!sla) sla = options->output;

		// Computing the length of the base name.
		length = dot - sla;
		// Limiting it to eight characters.
		if(length > 8) length = 8;

		// Copying the program name.
		strncpy(options->name, options->output, length);
		// Adding a terminating NUL character.
		options->name[length] = 0;
	}

	// Setting the default build date if no one was given.
	if(!*options->date)
	{
		// Using a raw time and a time structure pointer.
		time_t rawtime;
		struct tm *info;

		// Getting the raw time.
		time(&rawtime);
		// Getting time information from raw time.
		info = localtime(&rawtime);

		// Generating a date string from the structure informations.
		sprintf(options->date,"%04d.%02d%02d.%02d%02d",
			info->tm_year + 1900, info->tm_mon + 1, info->tm_mday,
			info->tm_hour, info->tm_min);
	}
}

/*
	generate()

	Generates a g1a header structure according to the given options.

	@arg	options	Options structure.
	@arg	data	Address of g1a header structure.
*/

void generate(struct Options options, char *data)
{
	// Using a predefined array for an unknown five-byte sequence.
	unsigned char unknown[5] = { 0x00, 0x10, 0x00, 0x10, 0x00 };

	// As there are many unknown or null areas in the header, initializing
	// all the data with zeros.
	memset(data, 0, 0x200);

	// Copying string "USBPower", that appears in system all files in the
	// calculator's file system.
	strcpy(data, "USBPower");
	// This flag indicates that the current file is an add-in.
	data[8] = 0xf3;
	// These five bytes appear insignificant.
	memcpy(data + 9, unknown, 5);
	// Skipping a checksum.

	// This byte role is quite unknown.
	data[0x00F] = 0x01;
	// Skipping a file size and a checksum.

	// These nine first bytes also seem insignificant, the last two
	// represent the number of objects in an MCS file (not interesting
	// here).
	memset(data + 15, 0, 11);

	// Here begins the add-in header.
	// Writing the application internal name.
	strncpy(data + 32, options.internal, 8);
	// Writing the number of e-strips (not handled here).
	data[43] = 0;
	// Writing the program version.
	strncpy(data + 48, options.version, 10);
	// Writing the build date.
	strncpy(data + 60, options.date, 14);
	// Writing the program icon.
	memcpy(data + 76, options.icon + 4, 68);
	// Skipping the e-strips data.

	// Writing the program name.
	strncpy(data + 468, options.name, 8);
	// Skipping a file size.
}

/*
	write()

	Write the header content to the output file, and then appends the
	contents of the input file.
	Also computes and writes total file size and checksums.

	@arg	input_file	Input binary file name.
	@arg	output_file	Output g1a file name.
	@arg	data		Header data address (casted as char *).
*/

void write(const char *input_file, const char *output_file, char *data)
{
	// Using input and output file pointers.
	FILE *input, *output;
	// Using an unsigned int to store file size.
	unsigned int size;
	// Using a byte.
	uint8_t byte;
	// Using an iterator.
	int i;

	// Opening input file.
	input = fopen(input_file, "rb");
	// Handling failure with a fatal error.
	if(!input) error_emit(FATAL, "input", input_file);

	// Opening output file.
	output = fopen(output_file, "wb");
	// Handling failure with a fatal error.
	if(!output)
	{
		// Closing the input file.
		fclose(input);
		// Emitting the fatal error.
		error_emit(FATAL,"output",output_file);
	}

	// Getting the total file size.
	fseek(input, 0, SEEK_END);
	// Adding 0x200 bytes for the g1a header.
	size = ftell(input) + 0x200;
	fseek(input, 0, SEEK_SET);

	// Computing the checksums (automatically truncated).
	data[0x00e] = size + 0x41;
	data[0x014] = size + 0xB8;
	// Writing the file size at offsets 0x010 and 0x1f0.
	for(i=0; i<4; i++)
	{
		// Getting a byte in the file size (this cast does not break
		// the strict aliasing rule).
		byte = *(((uint8_t *)&size) + 3 - i);
		// Writing the current byte in the data.
		data[0x010 + i] = data[0x1f0 + i] = byte;
	}

	// Inverting the MCS standard header.
	for(i=0; i < 0x020; i++) data[i] = ~data[i];
	// Writing the header to the file.
	fwrite(data, 1, 0x200, output);

	// Copying binary data.
	while(fread(&byte, 1, 1, input)) fwrite(&byte, 1, 1, output);

	// Closing the input and output files.
	fclose(input);
	fclose(output);
}

/*
	sring_format()

	This simple function checks if a string matches the given fixed-length
	format.
	If the format is shorter than the string (and the format matches all
	the beginning of the string), the string matches.

	List of special characters :
	-  'a' = [a-z]
	-  'A' = [A-Z]
	-  '0' = [0-9]
	-  '*' = everything printable
	Everything else is literal.

	@arg	string	String to test.
	@arg	format	Format to match.

	@return		0 if the string matches, 1 otherwise.
	
*/

int string_format(const char *str, const char *format)
{
	// Iterating over the characters.
	while(*str && *format)
	{
		// Handling the current character.
		switch(*format)
		{
			// Handling lowercase letters with islower().
			case 'a': if(!islower(*str)) return 1; break;
			// Handling uppercase letters with isupper().
			case 'A': if(!isupper(*str)) return 1; break;
			// Handling decimal digits with isdigit().
			case '0': if(!isdigit(*str)) return 1; break;
			// Handling printable characters with isprint().
			case '*': if(!isprint(*str)) return 1; break;
			// Handling anything else as literal.
			default: if(*str != *format) return 1; break;
		}

		// Incrementing the two pointer.
		str++;
		format++;
	}

	// At this point, the string matches, except if it's longer that the
	// format.
	return !*format;
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
	printf("Build date     '");
	ptr = data + 0x03c;
	// Printing the header characters and a line break.
	while(ptr < data + 0x04a && *ptr) putchar(*ptr++);
	puts("'\n");

	puts("Icon:");
	bitmap_output((uint8_t *)(data + 0x4c), 30, 19, stdout);
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
"\n\n"
"General options :\n"
"  -o   Output file name. Default is 'addin.g1a'.\n"
"  -i   Program icon, must be a valid non-indexed bmp file.\n"
"       Default is a blank icon.\n"
"  -n   Name of the add-in application. At most 8 characters.\n"
"       Default is the truncated output filename.\n"
"\n"
"Advanced options :\n"
"  --version=<text>   Program version. Format 'MM.mm.pppp' advised. Default\n"
"                     is '00.00.0000'.\n"
"  --internal=<name>  Internal name of the program. Uppercase and '@' at\n"
"                     beginning advised. Default is '@ADDIN'.\n"
"  --date=<date>      Date of the build, using format 'yyyy.MMdd.hhmm'.\n"
"                     Default is the current time.\n"
"\n"
"Other options :\n"
"  -h, --help           Displays this help.\n"
"      --info           Displays header format information.\n"
"  -d                   Display informations about a g1a file.\n"
"\n\n"
"You may also disable some warnings or errors during program execution.\n"
"However, disabling errors is strongly discouraged.\n"
"\n"
"Warning options :\n"
"  -Wlength       One of the parameters is too long and will be truncated.\n"
"  -Wformat       The parameter doesn't fit the default advised format.\n"
"  -Wbmp-width    The icon does not have the expected width.\n"
"  -Wbmp-height   The icon does not have the expected height.\n"
"  -Wbmp-color    The bitmap icon is not absolutely blank-and-white.\n"
"\n"
"Error options :\n"
"  -Eoption       Unrecognized option found.\n"
"  -Eillegal      Illegal invocation syntax (unexpected option found).\n"
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
