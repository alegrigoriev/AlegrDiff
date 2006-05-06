TODO:

1. When I hide a file from the comparison list, I expect the current selection to stay near (previous or next file, whatever), not to jump to the first file in the list. 
2. Add a pop-up menu item to rescan the differences while in file comparison mode. 
1. Please save viewing preferences. If I select two-pane view for one pair of files, I expect to have the same view when opening the next file pair. 
2. Add an option to automatically hide certain files (the same, right-only etc.) 
3. Can you lower task priority when scanning files - the computer stalls! 

Save As UNICODE file: set default TRUE in NT OS.
1. If a directory does not exist, no error message is given 
2. Show number of identical and different files in the status line.
Show confirmation dialog if file comparision or fingerprint calculation is canceled.

Open and read several files (up to one directory) at once
Make icons for the dialogs
Copy binary dump as text
Enable/disable OK in GOTO dialog for binary view
Add option for file size format (in kilobytes or in bytes).
Customizable number of MRU items
Make more robust argorithm of version info detection (if it is version line, don't include version data to a normalized CRC).
Detect Log: (as comment sequence)

Allow doublequotes in the search pattern
Assign an index to the files during original sort (for faster sorting)
Use OnActivateApp to refresh the files
Make sure to report read errors and file unaccessible errors during fingerprint creation.
make word by word comparison in MatchStrings
Make HTML help

Maybe different color for selected accepted/declined
add option to keep the caret on one line during F3 and F7
Add "Window\Close All" menu.

V1.5:
Need a hot-key to select current changed block
Detect moved text blocks
Compare 3 files for merge
Consider "signed fingerprint file" (UNICODE only)
Compare EXE version info for DLL, EXE, SYS
Show file version (add handler code)
Add Save differences
process linux-style MD5 signature files
// for text editor:
Ctrl+F8 - hold anchor, select lines
F8 - hold anchor

Problems:
Compare after refresh doesn't show progress. If compare was aborted and refresh clicked, estimated time is not shown. The files get read again for an MD5.
Directory share name cannot be selected if no '\' appended
If the text comparison got aborted because of read error, this is not shown
Blank line immediately following difference block is considered separate 
   for "Next Difference" command
Blank lines prevent from proper line matching inside difference blocks
Directory dialog allows to select network host. It won't return anything then.
Estimated time left is not shown while calculating a fingerpring of a big file  (???)

Fixed:
Vertical scroll to end on large files: file end may not be shown.
Ctrl+End on large files: may not go to end.
Vertical scrollbar may not be shown 
If one directory is dropped, the comparison starts by itself
"C:\\" shown in the list view title when checking a fingerprint
Doesn't refresh if one file changed on list view refresh
Fix tested:
Focus after hiding files
"Length" and Modification Time column On/Off not saved
History gets lost under Win9x
Text gets wrong colors (video driver problem)
Text clipping in split mode: line is not clipped
Zero byte in text file is causing text not shown
If the comparison is very quick, the progress dialog may get stuck
If the comparison got aborted because of read error, this is not shown
Cannot switch to text mode, after fingerprint check
Inconsistency between create and check fingerpring dlg
Wrong title in Creating Fingerprint dialog
Creates Recent File List subkey and deletes it on exit
"Different only in version stamp" changes to "Files are DIFFERENT" 
 when the file is opened.
After double click on listview item, the focus moves to the next item
In Windows 9x, binary comparison didn't work (FILE_SHARE_DELETE)
Selection lost when the list view is rebuilt
Last folder is not selected automatically in FileDialogWithHistory save
OK doesn't close the directory dialog, if a directory not found
During binary comparison, alternate "Calculating fingerprint" and "Comparing" messages shown
Subdirectory check fails when checking the fingerprint (???).
"   if()" and "  if ()" shows non-whitespace difference	- need full word mode
Title of the file diff includes extra '\'
Crash in the release build
When checking fingerprint, always showed "files identical"
Windows ME: old style file open dialog
Default sort for names: reversed
status string on open files: garbled
Name dialog doesn't work if directory name with backslash is specified
File dialog resize doesn't work if there is no _visible_ items below the separator
If fingerprint: "File Exists only in """
Scroll back in binary view redraws the whole view
Wrong arrow direction for name/dir sort
Status line for single file shows "Files DIFFERENT"
Non-UNICODE OS: wrong status string
Non-UNICODE OS: clipboard
File Date/time not shown in non-UNICODE OS
Non-UNICODE OS: doesn't work configuration dialog.
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

Retracted:
screen move in two directions may corrupt the image (??)

Done:

Don't show files from sub directories not in other dir (default).
Read files in alphabetical order (not reverse). 
File\Preferences dialog must remember selected tab for current session.
GoTo command for binary view
Increased number of MRU items to 30
Show/hide file classes
With binary comparison, progress update is done more often
Difference in file size in bytes reported in comparson result.
Side-by-side viewing for binary files
It shows the dialog with parameters instead of running the comparison for 
 the folders specified in a command line only if Shift is held
File size in the saved list.
Enable/disable "selected files" radio button in the Save List dialog
"Files different in version only" checkbox in the Save List dialog
Saving comma-separated file list
Add folder picture under the fingerprint picture
Find full word only
In 2-pane mode, go to another pane when added/removed text encountered
Big-endian UNICODE files supported
View files side by side
Add Copy 1st file text, copy 2nd file text
Save sizes of resizable dialogs for this session
Make File Compare dialog resizable
Make radio buttons in file compare dialog: Binary Text Default
Resync file list when compare result changed.
Add "Ignore folders" option
Must indicate time-consuming operations (like sorting a file list) with Hour Glass cursor.
Make progress dialog for binary comparision
Swap "File 2 Text" (V) "File 1 Text" (X) buttons in toolbar.
Hide files: Ctrl+H
Use dotted filenames in the progress dialog
Remove file result (change to "Ready" when the file window is deactivated)
Switch from binary view to text view
Switch from text view to binary view
When the binary file is open and its comparison result is unknown, start difference search from the beginning
Make Ctrl+A - select all in the list view
If no files found, show message box and close the view
replace edit box with a combobox in FolderDialog
Version stamp keywords: $Archive: $Revision: $Date:	and others
Add history to SaveListOf Files
When checking the fingerprint, use "Original" and "Current" instead of "1st and 2nd
Close file view if the file is updated in the list view
Update comparison result in the list view on load and reload
Remove final dialog in fingerprint creation
If a file(s) exists only in one directory, don't show "Copy From" for another directory
Don't change current directory
Add "CStringHistory" class
Set fingerprint list view title
Set file from fingerprint view title as single file only
Don't open a file which only exists in the fingerprint
Set binary view for fingerprinted file with one name only
Open all files from fingerprint list as binary
Test if a file from the fingerprint check list can be opened
Show subdirectories, if fingerprint is being checked
Change sort order and appearance of subdirs
Show sort order arrows with comctrl version < 6.0
If main or child window is minimized, save its state before minimization
Refresh list view
In Properties dialog: show file size
Add 1st file size, 2nd file size (with exact size tooltips?)
Make "Show columns" menu with "reset Columns"
Save columns width and order
Sort all result codes separately
For binary comparison: show file size
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
