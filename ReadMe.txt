TODO:
Open directories or files from the command line
Cancel selection on Accept/Decline commands (to see the result immediately). ???
Maybe different color for selected accepted/declined
Add Save differences
Add save file list
Make search dialog
Make Go To Line to first or second file line number
Import font settings from Visual C/VisualStudio
Add progress to File list view and file diff view
Make binary comparision
Compare EXE version info for DLL, EXE, SYS
Make progress dialog for binary comparision
Make background thread for comparision
UNICODE file support
//show subdir-only names in the status bar during comparision

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
Make combobox for last file filters
in InvalidateRange, add for letter overlap.
Make Word Left, Word Right.
Show added whitespaces
Add color selection for background.
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
