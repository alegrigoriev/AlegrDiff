TODO:

Add Copy 1st file text, copy 2nd file text
Refresh list view
make word by word comparison in MatchStrings
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Import font settings from Visual C/VisualStudio
Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7
If main window is minimized, save its state before minimization
V1.5:
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Make progress dialog for binary comparision
UNICODE file support
Add Save differences
Add save file list
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
screen move in two directions may corrupt the image (??)

Fixed:
"First file as base" button didn't work
Done:

//////////////////////////////
Comparing files in binary mode:
1. Load directories
2. find which files do have a corresponding pair with the same size and calculate CRC32/CRC64/MD5 for them. First calculate for the first source directory and subdirectories, then for the second.
3. Compare MD5 hash

New state codes:
Reading first file
Reading second file