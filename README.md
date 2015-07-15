
                                  g1a-wrapper


g1a-wrapper is a simple utility program that generates g1a files from pure
compiled binary source code.

It has a few options that make it possible to customize file header and many
information such as application name and version, icon, etc.



	Building and installing

To build g1a-wrapper from sources, just enter a terminal, move to the directory
containing the file 'Makefile' (and probably this one too), and use make :

	$ make all

Once you're done, you may install g1a-wrapper to your computer if you have
administrative rights :

	$ sudo make install

If so, you may then remove all this folder and the files it contains.



	Usage

If you have installed g1a-wrapper, it has moved to /usr/bin, so you can invoke
the program just by typing its name :

	$ g1a-wrapper <bin_file> [options...]

Otherwise, and if the executable file is not in the PATH (you may check it by
typing `echo $PATH` in a terminal and see if the containing folder appears),
you will have to use its absolute path :

	$ /path/to/executable/g1a-wrapper <bin_file> [options...]



	Bug report and improvements

Please report any bug or improvement idea to my git repository on bitbucket.org
at https://bitbucket.org/Lephenixnoir/add-in-wrapper/issues.
