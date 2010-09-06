To zip files:
It is recommended we dont use this, use zip -0 or jar -0 to store files.
zipper -w files.lst out.zip

file.lst contains a list of file one file per line.
when zipping the files in the list should be in the working directory.

To unzip files:
zipper [-r] foo.zip

This will unzip the files in foo.zip to the current directory.

-r will remove the foo.zip after unzipping.

The input files must be used with -0 or store only since zipper will
not inflate, deflated files.
