TODO:
Add color selection for background.
CaretRightToWord
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
Add Save differences
Add save file list
show subdir-only names in the status bar during comparision
Make Word Left, Word Right.
Make search dialog
Make Go To Line to first or second file line number
in InvalidateRange, add for letter overlap.
Make combobox for last file filters
Add progress to File list view and file diff view
Open directories or files from the command line
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Make background thread for comparision
UNICODE file support

Problems:
Blank lines are ignored during comparision, but may not be inored when line pair list built

Fixed:
Go To next differences when whitespaces ignored
when checking for alpnanum, check also for '_'
invalidates too much when cursor moved after scroll
Identical lines in the beginning of the file may be shown different
doesn't erase line ends after scrolling
If two files are completely different, only first file is shown
BIG file takes too much to read. Eliminated array reallocation.
File with differences: last lines not drawn
Wrong status for inserted and removed parts of strings

Done:
Hide Apply in the preferences sheet
Add Copy files
Ignore whitespace-only differences
Add Merge function
If a file reloaded and some changes are marked, ask confirmation.
don't cancel selection if right click inside the selection
Paint accepted and declined changes with color
Add "Number of lines to match", "Min string length to match", "percents of look-like difference"
Check difference area of the file for identical lines
Make Binary compare checkbox for file compare dialog
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
