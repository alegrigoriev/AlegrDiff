TODO:
Make Word Left, Word Right, Word selection.
Make search dialog
Make Go To Line to first or second file line number
Check difference area of the file for identical lines
in InvalidateRange, add for letter overlap.
Make Binary compare checkbox for file compare dialog
Make combobox for last file filters
Add progress to File list view and file diff view
Add Merge function
Add Save differences
Add save file list
Add Copy files
Open directories or files from the command line
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Make background thread for comparision
UNICODE file support

Problems:

Fixed:
Identical lines in the beginning of the file may be shown different
doesn't erase line ends after scrolling
If two files are completely different, only first file is shown
BIG file takes too much to read. Eliminated array reallocation.
File with differences: last lines not drawn
Wrong status for inserted and removed parts of strings

Done:
Make MRU folders list for file compare dialog
If folder not found, do dialog, destroy doc
Make Go To Line
Add context menu to file view
Add context menu to list view
Check file dates for change, when the app is activated
Open files for editing in file list view
Secondary sort after comparision result sort
Word selection
Find word or selection
allow files with only LF delimiters
Open files for editing
Auto reload for file view
Add "Refresh" function
Don't show "Subdirectory" column if not comparing subdirs
Show cursor position in the status bar
Draw selection for the single file
Add Line Numbers at the left margin (add for single file)
Added buffering to file read
Remember Used file filter 
Add font selection
Expand tabs during file loading
Make "Copy to clipboard"
Make "Home", "End" functions
Make "Move to next diff", Move to prev diff functions
Draw selection in the file view
Use standard background for drawing text
Add Options dialog
File pattern support in the dialog.
Allow two-file comparision
Open list view only after directory names are entered
Show correct title for list view
Show title for file view
Add full comparision result in list view and sorting
Allow multiple views for file pair (reference count ??)
OnWindowView must duplicate file view only
Process Enter key in list view
File view with line-wize differences
File view with in-line differences
