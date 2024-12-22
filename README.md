AlegrDiff - compare files and directories
-----------------------------------------

AlegrDiff is a file and directory comparison and fingerprinting program.

Development of AlegrDiff begun in 2001, as a replacement for Microsoft WinDiff application,
which had certain shortcomings.

AlergDiff can compare text and binary files.
Text files can be in 8 bit and UTF-16 (Unicode) encodings.

It can use a file digest (hash) for binary file comparison. It can also generate a fingerprint file
with hashes of all files in a directory, which you can use later to check if the files match those hashes.

With text file comparison, it can do a rudimentary merge by selecting which difference will go in a resulting file.
It's not same as resolving merge conflicts in a Git merge or rebase; here AlegrDiff is no help.

The program can be built with Microsoft Visual Studio 2019 or 2022.
It uses Microsoft Foundation Classes (MFC) libraries and headers,
which need to be installed with the Visual Studio installation.

It also builds a template for its help file, which, unfortunately, haven't been filled with any usable help.
