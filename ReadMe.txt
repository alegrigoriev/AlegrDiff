TODO:

Add extensions from the recent files to the browse for file dialog
Open Binary file difference
Support comparison of BIG lists
Add Copy 1st file text, copy 2nd file text
For binary comparison: show file size
Add 1st file size, 2nd file size (with exact size tooltips?)
Refresh list view
make word by word comparison in MatchStrings
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Import font settings from Visual C/VisualStudio
Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7
If main window is minimized, save its state before minimization

V1.5:
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Make progress dialog for binary comparision
Add Save differences
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
During binary comparison, alternate "Calculating fingerprint" and "Comparing" messages shown
screen move in two directions may corrupt the image (??)

Fixed:
Wrong BuildFilePairList arguments
"*." filter doesn't work
insufficient space allocated for copying files
"First file as base" button didn't work

Done:
Remove "Use Binary filter", "Use Ignore Filter" etc checkboxes,
  replace those edit controls with comboboxes
Compare directory structure
UNICODE file support
Choose format to save a merged file
Place marker when writing UNICODE file
Unicode file starts with 0xFEFF
Save all strings to history lists
Make binary comparision
Add save file list
Add "Hide" to list view context menu

//////////////////////////////
Comparing files in binary mode:
1. Load directories
2. find which files do have a corresponding pair with the same size and calculate CRC32/CRC64/MD5 for them. First calculate for the first source directory and subdirectories, then for the second.
3. Compare MD5 hash

New state codes:
Reading first file
Reading second file