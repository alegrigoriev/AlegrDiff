TODO:

Save columns width and order
Make "Show columns" menu with "reset Columns"
Add 1st file size, 2nd file size (with exact size tooltips?)
For binary comparison: show file size
Add "CStringHistory" class
Make sure to report read errors and file unaccessible errors during fingerprint creation.
In Properties dialog: show file size
Refresh list view
Add Copy 1st file text, copy 2nd file text
Copy binary dump as text
Switch from text view to binary view
Add folder picture under the fingerprint picture
Sort all result codes separately
Update comparison result on load and reload
replace edit box with a combobox in FolderDialog
Add history to SaveListOf Files

Detect "Version control information different only"
Detect "Different in spaces only"
make word by word comparison in MatchStrings
show version of file1, file2 of the line
Show Left-only, right-only, merged line (single line only)
Import font settings from Visual C/VisualStudio
Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7
If main window is minimized, save its state before minimization

V1.5:
Consider "signed fingerprint file" (UNICODE only)
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Make progress dialog for binary comparision
Add Save differences
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
Non-UNICODE OS: doesn't work configuration dialog.
Non-UNICODE OS: wrong status string
Non-UNICODE OS: clipboard
Blank lines prevent from proper line matching inside difference blocks
"   if()" and "  if ()" shows non-whitespace difference
Directory dialog allows to select network host. It won't return anything then.
Subdirectory check fails when checking the fingerprint.
During binary comparison, alternate "Calculating fingerprint" and "Comparing" messages shown
screen move in two directions may corrupt the image (??)

Fixed:
Crash when a folder is specified as file name and Browse clicked
Binary view: last bytes wrong
Wrong byte order in text representation, when words are shown
Crash when openng single binary file
text files with long lines make it crash (in text mode comparison).
GetFileData breaks sometimes (when movind data down in the buffer)
Cursor up out of screen
Click on the selection margin was broken.
TRACE statements with %s format converted to _T string
Wrong BuildFilePairList arguments
"*." filter doesn't work
insufficient space allocated for copying files
"First file as base" button didn't work

Done:
Check that the correct OleInitialize succeeded. If not, don't call OleUninitialize.
Show replace fingerprint file warning
Show progress inside one file
Check fingerprint
Horizontal scroll in binary view scroll data bytes only
Set line size for binary view
Change short tooltip to Create Fingerprint, Check fingerprint
Add toolbar buttons for Create Fingerprint, Check Fingerprint.
Mouse click in binary data chars.
Selection in binary data chars
Cursor in binary data chars
Click and mouse move beyond the area
Show data chars for binary file
Single file pair binary comparison
If one file is longer, show its data rather than '??'
Goto next/prev difference in binary view
Open Binary file difference
Address granularity is word size. Selection granularity too.
Add an icon for binary files' MDI child
Make "Show 1st, show 2nd for binary files
Enable/Disable OK in CheckDirFingerprint dialog
Enable/Disable OK in CreateDirFingerprint dialog
Set binary compare doc title
Add extensions from the recent files to the browse for file dialog
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

New state codes:
Reading first file
Reading second file

When list view is created, subitems are numbered in contiguous order (1, 2, 3, 4,5) as the columns are added
When a column is removed, subitem numbers are changed.
When a column is moved, subitems are reordered.
When a column is added, its subitem number is the last.
When a header item is clicked, use subitem number to convert to the column type
